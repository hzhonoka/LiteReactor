#include "net/InetAddress.h"
#include <netinet/in.h>
#include <strings.h>   // bzero
#include <arpa/inet.h> // inet_pton, inet_ntop, ntohs
#include <sys/socket.h>

InetAddress::InetAddress(uint16_t port) {
    bzero(&addr_, sizeof(addr_));           // 1. 把 addr_ 清零
    addr_.sin_family = AF_INET;               // 2. IPv4
    addr_.sin_addr.s_addr = htonl(INADDR_ANY);   // 3. 监听任意 IP
    addr_.sin_port = htons(port);          // 4. 端口转网络字节序
}

InetAddress::InetAddress(const std::string& ip, uint16_t port) {
    bzero(&addr_, sizeof(addr_));
    addr_.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);  // 5. 字符串 IP 转网络字节
    addr_.sin_port = htons(port);
}

std::string InetAddress::toIp() const {
    char buf[64] = "";
    inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));  // 网络字节转字符串
    return buf;
}

std::string InetAddress::toIpPort() const {
    return toIp() + ":" + std::to_string(toPort());  // 6. 端口转字符串
}

uint16_t InetAddress::toPort() const {
    return ntohs(addr_.sin_port);  // 7. 网络字节序转主机字节序
}