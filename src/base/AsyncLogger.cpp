#include "base/AsyncLogger.h"
#include <stdio.h>
#include <unistd.h>

AsyncLogger::AsyncLogger(const std::string& basename)
    : running_(false),
      basename_(basename),
      currentBuffer_(new Buffer),
      nextBuffer_(new Buffer)
{
    buffers_.reserve(16);
}

AsyncLogger::~AsyncLogger() {
    if (running_) stop();
}

void AsyncLogger::start() {
    running_ = true;
    thread_ = std::thread(&AsyncLogger::threadFunc, this);
}

void AsyncLogger::stop() {
    running_ = false;
    cond_.notify_one();
    thread_.join();
}

void AsyncLogger::append(const char* logline, size_t len) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (currentBuffer_->avail() > static_cast<int>(len)) {
        // 当前 buffer 够写
        currentBuffer_->append(logline, len);
    } else {
        // 当前 buffer 满了，push 到待写队列
        buffers_.push_back(std::move(currentBuffer_));
        
        if (nextBuffer_) {
            // 有备用 buffer，拿过来用
            currentBuffer_ = std::move(nextBuffer_);
        } else {
            // 备用也没了，new 一个（极少发生）
            currentBuffer_.reset(new Buffer);
        }
        
        currentBuffer_->append(logline, len);
        cond_.notify_one();  // 叫醒后端线程
    }
}

void AsyncLogger::threadFunc() {
    // 后端线程：两个本地 buffer
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    
    FILE* fp = fopen((basename_ + ".log").c_str(), "a");
    
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // 等 3 秒，或被前端叫醒
            cond_.wait_for(lock, std::chrono::seconds(flushInterval_));
            
            // 把前端的 buffer 全部拿过来
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            
            if (!nextBuffer_) {
                nextBuffer_ = std::move(newBuffer2);
            }
        }  // 解锁！
        
        // 写入文件（在锁外，不阻塞前端）
        for (const auto& buffer : buffersToWrite) {
            fwrite(buffer->data(), 1, buffer->length(), fp);
        }
        
        fflush(fp);
        
        // 回收 buffer
        if (buffersToWrite.size() > 2) {
            buffersToWrite.resize(2);
        }
        
        if (!newBuffer1) {
            newBuffer1 = std::move(buffersToWrite[0]);
            newBuffer1->reset();
        }
        if (!newBuffer2) {
            newBuffer2 = std::move(buffersToWrite[1]);
            newBuffer2->reset();
        }
        
        buffersToWrite.clear();
    }
    
    // 退出前把剩余日志刷盘
    fflush(fp);
    fclose(fp);
}