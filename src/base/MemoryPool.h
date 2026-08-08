#pragma once
#include <vector>
#include <cstddef>

class MemoryPool {
public:
    // blockSize: 每个块的大小（比如 64）
    explicit MemoryPool(size_t blockSize);
    ~MemoryPool();

    // 分配一块内存
    void* allocate();

    // 归还一块内存
    void deallocate(void* p);

    // 每个块的大小
    size_t blockSize() const { return blockSize_; }

private:
    // 申请一个大 chunk（4KB），切成小块加入自由链表
    void allocateChunk();

    struct Block {
        Block* next;  // 下一个空闲块（只在前8字节用）
    };

    size_t blockSize_;      // 块大小
    Block* freeList_;       // 自由链表头
    std::vector<void*> chunks_;  // 记录所有申请的大块，方便析构时释放
    static constexpr size_t kChunkSize = 4096;  // 每次申请 4KB
};