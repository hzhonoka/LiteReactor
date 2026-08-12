#include "net/EventLoop.h"
#include <iostream>
#include <thread>

int main() {
    EventLoop loop;
    
    // 测试：在 loop 线程里执行
    loop.runInLoop([]{
        std::cout << "In loop thread\n";
    });
    
    // 测试：从其他线程唤醒
    std::thread t([&loop]{
        std::this_thread::sleep_for(std::chrono::seconds(1));
        loop.runInLoop([]{
            std::cout << "From other thread\n";
        });
    });
    
    // loop 跑 2 秒后退出
    std::thread quitThread([&loop]{
        std::this_thread::sleep_for(std::chrono::seconds(2));
        loop.quit();
    });
    
    loop.loop();  // 阻塞在这里
    
    t.join();
    quitThread.join();
    return 0;
}