#include "net/InetAddress.h"
#include <iostream>

int main() {
    InetAddress addr1(8080);
    std::cout << "任意IP: " << addr1.toIpPort() << "\n";
    
    InetAddress addr2("127.0.0.1", 9090);
    std::cout << "指定IP: " << addr2.toIpPort() << "\n";
    
    return 0;
}