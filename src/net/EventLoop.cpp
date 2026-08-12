#include "net/EventLoop.h"
#include "net/Poller.h"
#include "net/Channel.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>

// 获取当前线程 ID（Linux 下）
pid_t gettid() {
    return static_cast<pid_t>(syscall(SYS_gettid));
}

EventLoop::EventLoop()
    : poller_(new Poller(this)),
      wakeupFd_(eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC)),
      looping_(false),
      quit_(false),
      threadId_(gettid())
{
    if (wakeupFd_ < 0) {
        std::cerr << "Failed in eventfd\n";
        abort();
    }
    
    // 创建 wakeupChannel，绑定到 wakeupFd
    wakeupChannel_.reset(new Channel(this, wakeupFd_));
    wakeupChannel_->setReadCallback(
        std::bind(&EventLoop::handleRead, this)
    );
    wakeupChannel_->enableReading();  // 注册到 epoll
}

EventLoop::~EventLoop() {
    wakeupChannel_->disableAll();
    wakeupChannel_->remove();  // 从 poller 移除
    ::close(wakeupFd_);
}

void EventLoop::loop() {
    looping_ = true;
    quit_ = false;

    while (!quit_) {
        activeChannels_.clear();
        poller_->poll(10000, &activeChannels_);  // 超时时间，比如 10000ms
        
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();  // 处理事件
        }
        
        doPendingFunctors();  // 执行其他线程投递的任务
    }

    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if (!isInLoopThread()) {
        wakeup();  // 如果不在 loop 线程，唤醒它让它退出
    }
}

bool EventLoop::isInLoopThread() const {
    return threadId_ == gettid();  // 当前线程 ID
}

void EventLoop::runInLoop(std::function<void()> cb) {
    if (isInLoopThread()) {
        cb();  // 直接执行
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(std::function<void()> cb) {
    {
        std::lock_guard<std::mutex> lock(mutex_);  // 保护 pending functors
        pendingFunctors_.push_back(std::move(cb));
    }
    
    if (!isInLoopThread()) {
        wakeup();  // 唤醒 loop 线程
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeupFd_, &one, sizeof(uint64_t));  // 写多少字节
}

void EventLoop::handleRead() {
    uint64_t one;
    ssize_t n = read(wakeupFd_, &one, sizeof(uint64_t));  // 读多少字节
}

void EventLoop::doPendingFunctors() {
    std::vector<std::function<void()>> functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);  // 交换，减少锁持有时间
    }
    
    for (const auto& f : functors) {
        f();  // 执行回调
    }
}

void EventLoop::updateChannel(Channel* channel) {
    poller_->updateChannel(channel);  // 转发给 Poller
}

void EventLoop::removeChannel(Channel* channel) {
    poller_->removeChannel(channel);  // 转发给 Poller
}