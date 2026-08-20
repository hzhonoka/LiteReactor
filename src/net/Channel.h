#pragma once
#include "base/noncopyable.h"
#include <functional>
#include <sys/epoll.h>

class EventLoop;  // 前向声明（明天讲）

class Channel : noncopyable {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent();  // 根据 revents_ 调用回调

    // 设置回调
    void setReadCallback(EventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    // 启用/禁用事件
    void enableReading() { events_ |= EPOLLIN; update(); }  // 加上 EPOLLIN
    void disableAll() { events_ = 0; update(); }      // 清零
    void enableWriting() { events_ |= EPOLLOUT; update(); }
    void disableWriting() { events_ &= ~EPOLLOUT; update(); }
    bool isWriting() const { return events_ & EPOLLOUT; }
    void remove();

    int fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }

    // 是否正在监听（用于 Poller 判断 ADD 还是 MOD）
    bool isNoneEvent() const { return events_ == 0; }

private:
    void update();  // 通知 Poller 更新本 Channel

    EventLoop* loop_;           // 所属 EventLoop
    const int fd_;              // 封装的 fd
    int events_;                // 关心的事件（传给 epoll）
    int revents_;               // 实际发生的事件（epoll 返回）

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};