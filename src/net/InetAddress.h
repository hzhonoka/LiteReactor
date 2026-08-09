#pragma once
#include <netinet/in.h>
#include <string>

class InetAddress {
public:
    // 监听任意 IP，只指定端口
    explicit InetAddress(uint16_t port = 0);
    
    // 指定 IP 和端口
    InetAddress(const std::string& ip, uint16_t port);
    
    // 从已有的 sockaddr_in 构造（accept 返回客户端地址时用）
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    
    // 返回底层地址，给 bind/accept/connect 用
    const sockaddr* getSockAddr() const { 
        return reinterpret_cast<const sockaddr*>(&addr_); 
    }
    
    // 转成人类可读的字符串
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

private:
    sockaddr_in addr_;  // 内部包着一个 C 结构体
};