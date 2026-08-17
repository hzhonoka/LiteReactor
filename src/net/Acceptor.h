#pragma once
#include "base/noncopyable.h"
#include "net/Socket.h"
#include "net/Channel.h"
#include <functional>

class EventLoop;
class InetAddress;

class Acceptor : noncopyable {
public:
    // 回调签名：新连接来了，传 connfd 和客户端地址
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);
    ~Acceptor();

    // 设置"新连接来了怎么办"的回调（TcpServer 会设置这个）
    void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    // 开始监听
    void listen();

private:
    // 对讲机响了，门卫执行这个
    void handleRead();

    EventLoop* loop_;              // 老板
    Socket acceptSocket_;          // 电话机（listenfd）
    Channel acceptChannel_;        // 对讲机（监听 listenfd 的事件）
    NewConnectionCallback newConnectionCallback_;  // "来了客人喊谁"
};