#pragma once
#include "base/noncopyable.h"
#include "net/InetAddress.h"

class Socket : noncopyable {
public:
    explicit Socket(int fd);
    ~Socket();
    
    Socket(Socket&& other) noexcept;
    Socket& operator=(Socket&& other) noexcept;
    
    int fd() const { return fd_; }
    
    void bind(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peerAddr);
    void setNonBlocking();
    void setReuseAddr(bool on);
    void shutdownWrite();

private:
    int fd_;
};