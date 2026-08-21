#include "net/TcpServer.h"
#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include <iostream>

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& name)
    : loop_(loop),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr)),
      started_(false),
      nextConnId_(1)
{
    // 给门卫塞纸条：来了客人，调用我的 newConnection
    acceptor_->setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this,
                  std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
    }
    acceptor_->listen();
}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    // 给新连接起名字：LiteReactor#1, LiteReactor#2...
    char buf[32];
    snprintf(buf, sizeof(buf), "#%d", nextConnId_++); //格式化成字符串 写入buf
    std::string connName = name_ + buf;
    
    std::cout << "newConnection [" << connName << "] from "
              << peerAddr.toIpPort() << "\n";
    
    // 本地地址（accept 返回的 fd 的本地地址）
    InetAddress localAddr;  // 简化版，先不填
    
    // #TODO: 多线程时从 ThreadPool 挑 loop，现在单线程用主 loop
    EventLoop* ioLoop = loop_;
    
    // 创建 TcpConnection
    TcpConnection::TcpConnectionPtr conn(
        new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    
    // 设置回调（上层给的）
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnection, this,
                  std::placeholders::_1));
    
    // 加入档案
    connections_[connName] = conn;
    
    // 服务员上岗！
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnection::TcpConnectionPtr& conn) {
    // 必须在主线程操作 connections_ map
    loop_->runInLoop(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnection::TcpConnectionPtr& conn) {
    // 从档案删除
    size_t n = connections_.erase(conn->name());
    (void)n;  // 避免警告
    
    // 在连接所属的 EventLoop 里销毁它
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(
        std::bind(&TcpConnection::connectDestroyed, conn));
}