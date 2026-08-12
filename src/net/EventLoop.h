#pragma once
#include "base/noncopyable.h"
#include <functional>
#include <vector>
#include <memory>
#include <mutex>

class Channel;
class Poller;

class EventLoop : noncopyable {
public:
    EventLoop();
    ~EventLoop();

    void loop();                    // 启动事件循环（阻塞）
    void quit();                    // 退出循环

    // 在当前 loop 线程执行回调（如果已在当前线程，直接执行）
    void runInLoop(std::function<void()> cb);
    
    // 把回调加入队列，唤醒 loop 线程执行
    void queueInLoop(std::function<void()> cb);

    void wakeup();                  // 唤醒 epoll_wait
    void updateChannel(Channel* channel);  // 转发给 Poller
    void removeChannel(Channel* channel);

    bool isInLoopThread() const;    // 判断是否在当前 loop 线程

private:
    void handleRead();              // 处理 wakeup eventfd 的可读事件
    void doPendingFunctors();       // 执行队列里的回调

    std::unique_ptr<Poller> poller_;        // Poller 对象
    int wakeupFd_;                          // eventfd
    std::unique_ptr<Channel> wakeupChannel_; // wakeupFd 的 Channel
    
    std::vector<Channel*> activeChannels_;  // 活跃的 Channel
    
    std::mutex mutex_;                      // 保护 pending functors
    std::vector<std::function<void()>> pendingFunctors_; // 待执行回调
    
    bool looping_;
    bool quit_;
    const pid_t threadId_;                  // 当前 loop 所属的线程 ID
};