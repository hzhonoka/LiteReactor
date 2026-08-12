#include "net/Poller.h"
#include "net/Channel.h"
#include <sys/epoll.h>
#include <unistd.h>

Poller::Poller(EventLoop* loop)
    : ownerLoop_(loop),
      epollfd_(epoll_create1(0)),  // epoll_create1
      events_(128)    // 初始容量
{
    if (epollfd_ < 0) {
        // 错误处理
    }
}

Poller::~Poller() {
    close(epollfd_);  // 关闭 epollfd
}

void Poller::poll(int timeoutMs, std::vector<Channel*>* activeChannels) {
    int numEvents = epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(events_[i].data.ptr);  // 从 epoll_event.data 取 Channel*
        channel->set_revents(events_[i].events);  // 设置实际发生的事件
        activeChannels->push_back(channel);
    }
    
    // 如果事件填满了，扩容
    if (static_cast<size_t>(numEvents) == events_.size()) {
        events_.resize(events_.size() * 2);
    }
}

void Poller::updateChannel(Channel* channel) {
    const int fd = channel->fd();
    
    if (channel->isNoneEvent()) {
        // 如果 Channel 不关心任何事件，从 epoll 删除
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr);
        channels_.erase(fd);  // 从 map 删除
    } else {
        struct epoll_event ev;
        ev.events = channel->events();
        ev.data.ptr = channel;  // 存 Channel* 指针！
        
        if (channels_.find(fd) == channels_.end()) {
            // 不在 map 中，新增
            epoll_ctl(epollfd_, EPOLL_CTL_ADD, fd, &ev);
            channels_[fd] = channel;
        } else {
            // 已在 map 中，修改
            epoll_ctl(epollfd_, EPOLL_CTL_MOD, fd, &ev);
        }
    }
}

void Poller::removeChannel(Channel* channel) {
    int fd = channel->fd();
    auto it = channels_.find(fd);
    if (it != channels_.end()) {
        epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, nullptr);
        channels_.erase(it);
    }
}