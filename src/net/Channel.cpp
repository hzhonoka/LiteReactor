#include "net/Channel.h"
#include <sys/epoll.h>
#include "net/EventLoop.h"

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0), revents_(0)
{
}

Channel::~Channel() {
    // Channel 不拥有 fd，不 close
}

void Channel::handleEvent() {
    if (revents_ & EPOLLHUP) {
        if (closeCallback_) closeCallback_();
    }
    if (revents_ & EPOLLERR) {  // 错误事件
        if (errorCallback_) errorCallback_();
    }
    if (revents_ & (EPOLLIN | EPOLLPRI)) {  // 可读
        if (readCallback_) readCallback_();
    }
    if (revents_ & EPOLLOUT) {  // 可写
        if (writeCallback_) writeCallback_();
    }
}

void Channel::update() {
    loop_->updateChannel(this);
}

void Channel::remove() {
    loop_->removeChannel(this);
}