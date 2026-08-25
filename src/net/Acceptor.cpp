#include "net/Acceptor.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include <unistd.h>
#include <iostream>

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr)
    : loop_(loop),
      acceptSocket_(::socket(AF_INET, SOCK_STREAM, 0)),  // 买一部电话
      acceptChannel_(loop, acceptSocket_.fd())             // 配一个对讲机，连到这部电话
{
    acceptSocket_.setReuseAddr(true);   // 端口复用
    acceptSocket_.bind(listenAddr);     // 绑定地址：我在 8080 号
    acceptChannel_.setReadCallback(
        std::bind(&Acceptor::handleRead, this));  // 对讲机响了，调用 handleRead
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();  // 对讲机关机
    acceptChannel_.remove();      // 从老板的花名册删除
}

void Acceptor::listen() {
    acceptSocket_.listen();       // 电话设为接听模式
    acceptChannel_.enableReading(); // 对讲机开机：有人敲门告诉我
}

void Acceptor::handleRead() {
    InetAddress peerAddr;  // 客人的地址
    int connfd = acceptSocket_.accept(&peerAddr);  // 接电话，拿到客人的分机号

    if (connfd >= 0) {
        if (newConnectionCallback_) {
            // 喊前台："来客人了，分机号是 connfd，地址是 peerAddr"
            newConnectionCallback_(connfd, peerAddr);
        } else {
            // 没人处理？直接挂掉
            ::close(connfd);
        }
    }
}