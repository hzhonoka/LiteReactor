#pragma once

class noncopyable {
public:
    noncopyable(const noncopyable&) = delete;            // 禁止拷贝构造
    noncopyable& operator=(const noncopyable&) = delete; // 禁止拷贝赋值
protected:
    noncopyable() = default;  // 允许子类构造
    ~noncopyable() = default; // 允许子类析构
};