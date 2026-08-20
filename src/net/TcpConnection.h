#pragma once
#include "base/noncopyable.h"
#include "net/InetAddress.h"
#include "base/Buffer.h"
#include <memory>
#include <functional>

class EventLoop;
class Socket;
class Channel;

class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
public:
    using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
    using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback = std::function<void(const TcpConnectionPtr&, Buffer*)>;
    using CloseCallback = std::function<void(const TcpConnectionPtr&)>;

    TcpConnection(EventLoop* loop, const std::string& name,
                  int sockfd, const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();

    // 发送数据（先攒到 outputBuffer，再触发写）
    void send(const std::string& message);

    // 关闭连接（半关闭：不再发数据，等对方关）
    void shutdown();

    // 设置回调（TcpServer 会设置这些）
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    void setCloseCallback(const CloseCallback& cb) { closeCallback_ = cb; }

    // 连接建立/销毁时由 TcpServer 调用
    void connectEstablished();
    void connectDestroyed();

    const std::string& name() const { return name_; }
    const InetAddress& localAddress() const { return localAddr_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };
    
    void setState(StateE s) { state_ = s; }

    void handleRead();   // Channel 可读回调
    void handleWrite();  // Channel 可写回调
    void handleClose();  // Channel 关闭回调
    void handleError();  // Channel 错误回调

    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop* loop_;
    std::string name_;
    StateE state_;

    std::unique_ptr<Socket> socket_;
    std::unique_ptr<Channel> channel_;

    InetAddress localAddr_;
    InetAddress peerAddr_;

    Buffer inputBuffer_;   // 读缓冲
    Buffer outputBuffer_;  // 写缓冲

    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
};