#include "base/MemoryPool.h"

MemoryPool::MemoryPool(size_t blockSize)
    : blockSize_(blockSize), freeList_(nullptr)
{
    // 块大小至少能放下一个指针（64位系统指针8字节），否则没法链起来
    if (blockSize_ < sizeof(Block*)) {
        blockSize_ = sizeof(Block*);
    }
    allocateChunk();  // 预先申请第一个 chunk
}

MemoryPool::~MemoryPool() {
    // 释放所有申请的大块内存
    for (void* chunk : chunks_) {
        delete[] static_cast<char*>(chunk);
    }
}

void MemoryPool::allocateChunk() {
    // 申请 4KB 内存
    char* chunk = new char[kChunkSize];
    chunks_.push_back(chunk);  // 记录，析构时释放

    size_t numBlocks = kChunkSize / blockSize_;
    
    // 从后往前串链表，这样 freeList_ 最终指向第一块
    for (size_t i = numBlocks; i > 0; --i) {
        char* block = chunk + (i - 1) * blockSize_;
        Block* current = reinterpret_cast<Block*>(block);
        current->next = freeList_;        // 这块的 next 指向原来的链表头
        freeList_ = current;            // 链表头更新为这块
    }
}

void* MemoryPool::allocate() {
    if (freeList_ == nullptr) {         // 没有空闲块了
        allocateChunk();             // 申请新 chunk
    }
    
    Block* block = freeList_;       // 取链表头
    freeList_ = freeList_->next;                // 链表头指向下一个
    return block;
}

void MemoryPool::deallocate(void* p) {
    if (p == nullptr) return;           // 空指针不处理
    
    Block* block = reinterpret_cast<Block*>(p);
    block->next = freeList_;              // 这块的 next 指向原来的链表头
    freeList_ = block;                // 链表头更新为这块
}