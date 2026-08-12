#pragma once
#include "base/noncopyable.h"
#include <vector>
#include <map>

class Channel;
class EventLoop;

class Poller : noncopyable {
public:
    explicit Poller(EventLoop* loop);
    ~Poller();

    // 调用 epoll_wait，活跃 Channel 填入 activeChannels
    void poll(int timeoutMs, std::vector<Channel*>* activeChannels);

    // 增删改 Channel 的 epoll 注册
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);

private:
    using ChannelMap = std::map<int, Channel*>;
    
    EventLoop* ownerLoop_;           // 所属 EventLoop
    int epollfd_;                    // epoll 实例
    std::vector<struct epoll_event> events_;  // epoll_wait 返回数组
    ChannelMap channels_;            // fd -> Channel* 映射
};