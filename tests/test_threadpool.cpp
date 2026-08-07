#include "ThreadPool.h"
#include <iostream>
#include <mutex>


int main() {
    std::mutex print_mtx;
    ThreadPool pool(4);  // 4 个线程
    
    for (int i = 0; i < 8; ++i) 
    {
        pool.execute([i, &print_mtx] 
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lock(print_mtx);
            std::cout << "Task " << i << " running in thread " 
                    << std::this_thread::get_id() << "\n";
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    pool.shutdown();
    return 0;
}