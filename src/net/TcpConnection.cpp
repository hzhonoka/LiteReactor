#include "net/TcpConnection.h"
#include "net/EventLoop.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include <unistd.h>
#include <errno.h>

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : loop_(loop),
      name_(name),
      state_(kConnecting),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(peerAddr),context_(nullptr)
{
    // 设置 Channel 的四个回调
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteCallback(
        std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(
        std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(
        std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection() {
    // Socket 是 unique_ptr，析构时自动 close(fd)
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->enableReading();  // 开始监听可读事件
    
    if (connectionCallback_) {
        connectionCallback_(shared_from_this());
    }
}

void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
    }
    channel_->remove();  // 从 Poller 删除
}

void TcpConnection::handleRead() {
    // 从 fd 读到 inputBuffer
    ssize_t n = inputBuffer_.readFd(channel_->fd());
    
    if (n > 0) {
        // 有数据，通知上层（比如 EchoServer）
        if (messageCallback_) {
            messageCallback_(shared_from_this(), &inputBuffer_);
        }
    } else if (n == 0) {
        // 对端关闭
        handleClose();
    } else {
        // 出错
        handleError();
    }
}

void TcpConnection::handleWrite() {
    if (channel_->isWriting()) {
        ssize_t n = write(channel_->fd(), outputBuffer_.peek(), 
                          outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                // 写完了，不再关心可写事件
                channel_->disableWriting();
            }
        }
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected);
    channel_->disableAll();
    
    if (closeCallback_) {
        closeCallback_(shared_from_this());
    }
}

void TcpConnection::handleError() {
    // 可以打日志
}

void TcpConnection::send(const std::string& message) {
    if (state_ == kConnected) {
        sendInLoop(message);
    }
}

void TcpConnection::sendInLoop(const std::string& message) {
    // 简化版：直接写，如果写不完再注册 EPOLLOUT
    ssize_t nwrote = write(channel_->fd(), message.data(), message.size());
    if (nwrote < 0) {
        nwrote = 0;
    }
    
    size_t remaining = message.size() - nwrote;
    if (remaining > 0) {
        // 没写完，放到 outputBuffer，注册可写事件
        outputBuffer_.append(message.data() + nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        shutdownInLoop();
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();  // 半关闭：不再发送，但还能收
    }
}