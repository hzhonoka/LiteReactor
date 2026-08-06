#pragma once
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <iostream>

class ThreadPool 
{
    public:
        // 构造函数：创建 threadCount 个工作线程
        explicit ThreadPool(size_t threadCount) : stop_(false)
        {
            for (size_t i = 0; i < threadCount; ++i) 
            {
                workers_.emplace_back([this] {
                // 每个工作线程的主循环
                    while (true) {
                        std::function<void()> task;
                        {
                            // 步骤 1：加锁，看队列
                            std::unique_lock<std::mutex> lock(mutex_);

                            // 步骤 2：等条件（有任务 或 要停止）
                            // 如果 stop_==false 且 tasks_空，就解锁并睡死
                            cv_.wait(lock, [this] {
                                return stop_ || !tasks_.empty();
                            });
                            
                            // 步骤 3：被叫醒后，检查是不是要退出
                            if (stop_ && tasks_.empty()) {
                                return;  // 线程结束
                            }
                            
                            // 步骤 4：取任务
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }  // 步骤 5：解锁（lock 出作用域自动销毁）
                        
                        // 步骤 6：执行任务（在锁外执行，避免阻塞其他线程取任务）
                        task();
                    }
                });
            }
        }

        ~ThreadPool()
        {
            shutdown();
        }

        void execute(std::function<void()> task) 
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (stop_) return;  // 如果线程池已停止，拒绝新任务
                tasks_.emplace(std::move(task));
            }
            cv_.notify_one();  // 叫醒一个工作线程
        }

        void shutdown() 
        {
            {
                std::unique_lock<std::mutex> lock(mutex_);
                stop_ = true;  // 设置停止标志
            }
            cv_.notify_all();  // 叫醒所有线程，让它们检查 stop_ 并退出
            
            for (auto& worker : workers_) {
                if (worker.joinable()) {
                    worker.join();  // 等待线程结束
                }
            }
        }

        size_t size() const { return workers_.size(); }

        
    private:
        std::vector<std::thread> workers_;        // 工作线程数组
        std::queue<std::function<void()>> tasks_; // 任务队列
        std::mutex mutex_;                        // 保护队列的锁
        std::condition_variable cv_;              // 条件变量，线程睡眠/唤醒
        bool stop_;                               // 停止标志
};

