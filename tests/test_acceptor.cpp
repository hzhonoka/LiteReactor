#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include <unistd.h>
#include <iostream>

int main() {
    EventLoop loop;                     // 老板
    InetAddress listenAddr(8080);       // 地址：0.0.0.0:8080
    Acceptor acceptor(&loop, listenAddr); // 雇一个门卫
    
    // 门卫说："来客人了，我就打印一下，然后挂电话"
    acceptor.setNewConnectionCallback([](int connfd, const InetAddress& peerAddr) {
        std::cout << "New connection from " << peerAddr.toIpPort() 
                  << ", fd = " << connfd << "\n";
        ::close(connfd);  // 测试：直接关掉
    });
    
    acceptor.listen();  // 门卫开始工作
    std::cout << "Acceptor listening on 8080...\n";
    
    loop.loop();  // 老板开始上班（阻塞在这里）
    return 0;
}