#include "net/Socket.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

Socket::Socket(int fd) : fd_(fd) {}

Socket::~Socket() {
    if (fd_ >= 0) {
        ::close(fd_);
    }
}

// 移动构造
Socket::Socket(Socket&& other) noexcept : fd_(other.fd_) {
    other.fd_ = -1;
}

// 移动赋值
Socket& Socket::operator=(Socket&& other) noexcept {
    if (this != &other) {
        if (fd_ >= 0) ::close(fd_);
        fd_ = other.fd_;
        other.fd_ = -1;
    }
    return *this;
}

void Socket::bind(const InetAddress& addr) {
    ::bind(fd_, addr.getSockAddr(), sizeof(sockaddr_in));
}

void Socket::listen() {
    ::listen(fd_, 128);
}

int Socket::accept(InetAddress* peerAddr) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int connfd = ::accept(fd_, reinterpret_cast<sockaddr*>(&addr), &len);
    if (connfd >= 0 && peerAddr != nullptr) {
        *peerAddr = InetAddress(addr);
    }
    return connfd;
}

void Socket::setNonBlocking() {
    int oldFlag = ::fcntl(fd_, F_GETFL);
    ::fcntl(fd_, F_SETFL, oldFlag | O_NONBLOCK);
}

void Socket::setReuseAddr(bool on) {
    int opt = on ? 1 : 0;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

void Socket::shutdownWrite() {
    if (::shutdown(fd_, SHUT_WR) < 0) {
        // 错误处理，可选
    }
}