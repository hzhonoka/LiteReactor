#include "net/Socket.h"
#include "net/InetAddress.h"
#include <iostream>

int main() {
    InetAddress addr(8080);
    
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    Socket sock(fd);
    sock.setReuseAddr(true);
    sock.bind(addr);
    sock.listen();
    
    std::cout << "fd: " << sock.fd() << "\n";
    
    // 移动测试
    Socket sock2(std::move(sock));
    std::cout << "sock2 fd: " << sock2.fd() << "\n";
    // sock.fd() 现在应该是 -1
    
    return 0;
}