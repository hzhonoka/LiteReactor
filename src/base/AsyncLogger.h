#pragma once
#include "base/noncopyable.h"
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <cstring>
#include <chrono>   

class AsyncLogger : noncopyable {
public:
    // 简化版：4KB 日志缓冲区
    class Buffer {
    public:
        Buffer() : cur_(data_) {}
        
        void append(const char* buf, size_t len) {
            if (static_cast<size_t>(avail()) > len) {
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        }
        
        const char* data() const { return data_; }
        size_t length() const { return cur_ - data_; }
        void reset() { cur_ = data_; }
        int avail() const { return static_cast<int>(end() - cur_); }
        
    private:
        const char* end() const { return data_ + sizeof(data_); }
        
        char data_[4000];  // 4KB
        char* cur_;
    };
    
    using BufferPtr = std::unique_ptr<Buffer>;
    using BufferVector = std::vector<BufferPtr>;

    explicit AsyncLogger(const std::string& basename);
    ~AsyncLogger();
    
    void start();  // 启动后端线程
    void stop();   // 停止后端线程
    void append(const char* logline, size_t len);  // 前端调用

private:
    void threadFunc();  // 后端线程主循环
    
    void flushBuffers(BufferVector& buffers);
    
    const int flushInterval_ = 3;  // 3秒超时刷新
    bool running_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::string basename_;
    
    BufferPtr currentBuffer_;   // 前端正在写的 buffer
    BufferPtr nextBuffer_;      // 备用 buffer
    BufferVector buffers_;      // 待写入文件的 buffer 列表
};