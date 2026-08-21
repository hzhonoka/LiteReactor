#pragma once
#include "base/noncopyable.h"
#include "net/InetAddress.h"
#include "net/TcpConnection.h"
#include <map>
#include <string>
#include <memory>

class EventLoop;
class Acceptor;

class TcpServer : noncopyable {
public:
    using ConnectionCallback = TcpConnection::ConnectionCallback;
    using MessageCallback = TcpConnection::MessageCallback;

    TcpServer(EventLoop* loop, const InetAddress& listenAddr,
              const std::string& name);
    ~TcpServer();

    void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    void start();

private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnection::TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnection::TcpConnectionPtr& conn);

    EventLoop* loop_;
    const std::string name_;
    std::unique_ptr<Acceptor> acceptor_;
    bool started_;
    int nextConnId_; //编号计数器 区分TcpConnection
    
    std::map<std::string, TcpConnection::TcpConnectionPtr> connections_;
    
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
};