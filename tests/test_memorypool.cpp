#include "base/MemoryPool.h"
#include <iostream>

int main() {
    MemoryPool pool(64);  // 64 字节一块
    
    void* p1 = pool.allocate();
    void* p2 = pool.allocate();
    void* p3 = pool.allocate();
    
    std::cout << "p1: " << p1 << "\n";
    std::cout << "p2: " << p2 << "\n";
    std::cout << "p3: " << p3 << "\n";
    std::cout << "p2 - p1 = " << (static_cast<char*>(p2) - static_cast<char*>(p1)) << " bytes\n";
    // 应该等于 64！
    
    pool.deallocate(p2);
    
    void* p4 = pool.allocate();
    std::cout << "p4 (reuse p2): " << p4 << "\n";
    // p4 应该等于 p2！
    
    return 0;
}