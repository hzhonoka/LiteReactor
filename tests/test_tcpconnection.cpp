#include "net/TcpConnection.h"
#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include <iostream>
#include <set>

std::set<std::shared_ptr<TcpConnection>> connections;

int main() {
    EventLoop loop;
    InetAddress listenAddr(8080);
    Acceptor acceptor(&loop, listenAddr);
    
    acceptor.setNewConnectionCallback([&loop](int connfd, const InetAddress& peerAddr) {
        std::cout << "New connection from " << peerAddr.toIpPort() 
                  << ", fd = " << connfd << "\n";
        
        // 创建 TcpConnection
        auto conn = std::make_shared<TcpConnection>(
            &loop, "conn" + std::to_string(connfd),
            connfd, InetAddress(8080), peerAddr);
        
        // Echo 回调：收到什么发回什么
        conn->setMessageCallback([](const TcpConnection::TcpConnectionPtr& conn,
                                    Buffer* buf) {
            std::string msg = buf->retrieveAllAsString();
            std::cout << "Received: " << msg;
            conn->send(msg);  // 回显
        });
        
        conn->setCloseCallback([](const TcpConnection::TcpConnectionPtr& conn) {
            std::cout << "Connection closed: " << conn->name() << "\n";
            connections.erase(conn);
        });
        
        conn->connectEstablished();
        connections.insert(conn);
    });
    
    acceptor.listen();
    std::cout << "Echo server on 8080...\n";
    loop.loop();
    return 0;
}