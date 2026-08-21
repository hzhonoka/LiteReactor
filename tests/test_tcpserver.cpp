#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include <iostream>

int main() {
    EventLoop loop;
    InetAddress listenAddr(8080);
    TcpServer server(&loop, listenAddr, "LiteReactor");
    
    server.setConnectionCallback([](const TcpConnection::TcpConnectionPtr& conn) {
        std::cout << (conn->connected() ? "UP" : "DOWN") 
                  << ": " << conn->peerAddress().toIpPort() << "\n";
    });
    
    server.setMessageCallback([](const TcpConnection::TcpConnectionPtr& conn,
                                  Buffer* buf) {
        std::string msg = buf->retrieveAllAsString();
        std::cout << "Received: " << msg;
        conn->send(msg);
    });
    
    server.start();
    std::cout << "TcpServer on 8080...\n";
    loop.loop();
    return 0;
}


