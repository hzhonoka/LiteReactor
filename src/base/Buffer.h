#pragma once
#include <vector>
#include <string>
#include <algorithm>

class Buffer {
public:
    static const size_t kCheapPrepend = 8;    // 预留头部空间
    static const size_t kInitialSize = 1024;  // 初始容量

    Buffer();

    // 可读/可写字节数
    size_t readableBytes() const;
    size_t writableBytes() const;
    size_t prependableBytes() const;

    // 读指针位置
    const char* peek() const;

    // 查找 \r\n（HTTP 解析用）
    const char* findCRLF() const;

    // 取走数据
    void retrieve(size_t len);
    void retrieveAll();
    std::string retrieveAllAsString();
    std::string retrieveAsString(size_t len);

    // 追加数据
    void append(const std::string& str);
    void append(const char* data, size_t len);

    // 确保可写空间足够
    void ensureWritableBytes(size_t len);

    // 写指针位置（供 read(fd, beginWrite(), writableBytes()) 用）
    char* beginWrite();
    void hasWritten(size_t len);

private:
    char* begin();                    // buffer_ 起始地址
    const char* begin() const;
    void makeSpace(size_t len);       // 扩容或移动数据

    std::vector<char> buffer_;        // 底层存储
    size_t readerIndex_;              // 读索引
    size_t writerIndex_;              // 写索引
};