#include "base/Buffer.h"
#include <iostream>

int main() {
    Buffer buf;
    
    std::cout << "初始可读: " << buf.readableBytes() << "\n";
    std::cout << "初始可写: " << buf.writableBytes() << "\n";
    
    buf.append("Hello");
    std::cout << "追加 'Hello' 后可读: " << buf.readableBytes() << "\n";
    
    buf.append(" World");
    std::cout << "追加 ' World' 后可读: " << buf.readableBytes() << "\n";
    
    std::string str = buf.retrieveAllAsString();
    std::cout << "取出的内容: [" << str << "]\n";
    std::cout << "取完后可读: " << buf.readableBytes() << "\n";
    std::cout << "取完后可写: " << buf.writableBytes() << "\n";
    
    // 测试再次追加
    buf.append("Ciallo");
    std::cout << "再次追加后可读: " << buf.readableBytes() << "\n";
    std::cout << "再次取出: [" << buf.retrieveAllAsString() << "]\n";
    
    return 0;
}