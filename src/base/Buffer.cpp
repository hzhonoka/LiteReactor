#include "base/Buffer.h"
#include <algorithm>
#include <sys/uio.h>
#include <errno.h>
#include <unistd.h>

Buffer::Buffer()
    : buffer_(kCheapPrepend + kInitialSize),  // vector 初始容量
      readerIndex_(kCheapPrepend),                       // 读从哪里开始？
      writerIndex_(kCheapPrepend)                        // 写从哪里开始？
{
}

// ========== 辅助函数：返回底层数组起始地址 ==========
char* Buffer::begin() {
    return buffer_.data();  // vector 的第一个元素的地址
}

const char* Buffer::begin() const {
    return buffer_.data();  // const 版本，不能修改
}

// ========== 可读/可写/可预留 字节数 ==========
size_t Buffer::readableBytes() const {
    return writerIndex_ - readerIndex_;  // 写指针 - 读指针
}

size_t Buffer::writableBytes() const {
    return buffer_.size() - writerIndex_;  // 总容量 - 写指针
}

size_t Buffer::prependableBytes() const {
    return readerIndex_;  // 读指针 - 0（前面有多少空闲）
}

// ========== 读指针位置 ==========
const char* Buffer::peek() const {
    return begin() + readerIndex_;  // 从数组起始 + readerIndex_
}

// ========== 查找 \r\n（HTTP 解析用）==========
const char* Buffer::findCRLF() const {
    const char* crlf = std::search(peek(), begin() + writerIndex_, "\r\n", "\r\n" + 2);
    // std::search 在 [peek(), begin()+writerIndex_) 里找 "\r\n"
    return crlf == (begin() + writerIndex_) ? nullptr : crlf;
}

// ========== 取走数据 ==========
void Buffer::retrieve(size_t len) {
    // 如果取走的比可读的多，就全取走
    if (len < readableBytes()) {
        readerIndex_ += len;  // 读指针往前移
    } else {
        retrieveAll();
    }
}

void Buffer::retrieveAll() {
    readerIndex_ = kCheapPrepend;  // 重置到初始位置
    writerIndex_ = kCheapPrepend;
}

std::string Buffer::retrieveAllAsString() {
    return retrieveAsString(readableBytes());  // 取走所有可读数据
}

std::string Buffer::retrieveAsString(size_t len) {
    std::string result(peek(),  len);  // 从 peek() 开始，取 len 字节
    retrieve(len);                     // 标记为已读
    return result;
    
}

// ========== 追加数据 ==========
void Buffer::append(const std::string& str) {
    append(str.data(), str.size());
}

void Buffer::append(const char* data, size_t len) {
    ensureWritableBytes(len);                          // 确保空间够
    std::copy(data, data + len, beginWrite());         // 拷贝数据到写指针位置
    hasWritten(len);                                    // 移动写指针
}

// ========== 确保可写空间 ==========
void Buffer::ensureWritableBytes(size_t len) {
    if (writableBytes() < len) {
        makeSpace(len);  // 空间不够，扩容或移动
    }
    // 断言：现在空间一定够
}

// ========== 写指针操作 ==========
char* Buffer::beginWrite() {
    return begin() + writerIndex_;  // 写指针位置
}

void Buffer::hasWritten(size_t len) {
    writerIndex_ += len;  // 写指针前移
}

// ========== 核心：扩容或移动数据 ==========
void Buffer::makeSpace(size_t len) {
    // 如果 前面空闲 + 后面空闲 够 len，就把数据往前移
    if (prependableBytes() + writableBytes() < len + kCheapPrepend) {
        // 连移都不够，只能扩容
        buffer_.resize(writerIndex_ + len);
    } else {
        // 前面有空闲，把 readable 数据移到前面
        size_t readable = readableBytes();
        std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
        readerIndex_ = kCheapPrepend;  // 移到预留头部后面
        writerIndex_ = readerIndex_ + readable;  
    }
}

ssize_t Buffer::readFd(int fd, int* savedErrno) {
    char extrabuf[65536];  // 栈上临时缓冲
    struct iovec vec[2];
    
    size_t writable = writableBytes();
    vec[0].iov_base = beginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);
    
    // readv：读到两个地方，防止 Buffer 不够
    int iovcnt = (writable < sizeof(extrabuf)) ? 2 : 1;
    ssize_t n = readv(fd, vec, iovcnt);
    
    if (n < 0) {
        if (savedErrno) *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writable) {
        hasWritten(n);
    } else {
        hasWritten(writable);
        append(extrabuf, n - writable);
    }
    return n;
}