#include "http/HttpContext.h"
#include <algorithm>

bool HttpContext::parseRequest(Buffer* buf) {
    bool ok = true;
    bool hasMore = true;

    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    buf->retrieveUntil(crlf + 2);  // 删掉请求行
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;  // 数据不够
            }
        } else if (state_ == kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* start = buf->peek();
                if (start == crlf) {
                    // 空行！Header 结束
                    buf->retrieveUntil(crlf + 2);  // 把空行也取走
                    state_ = kGotAll;
                    hasMore = false;
                } else {
                    // 解析一个 Header
                    const char* colon = std::find(start, crlf, ':');
                    if (colon != crlf) {
                        request_.addHeader(start, colon, crlf);
                    }
                    buf->retrieveUntil(crlf + 2);
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == kGotAll) {
            hasMore = false;
        }
    }
    return ok;
}

bool HttpContext::processRequestLine(const char* begin, const char* end) {
    // 找第一个空格：GET /index.html HTTP/1.1
    const char* space = std::find(begin, end, ' ');
    if (space == end) return false;

    // 解析方法
    if (!request_.setMethod(begin, space)) return false;

    // 找第二个空格
    const char* start = space + 1;
    space = std::find(start, end, ' ');
    if (space == end) return false;

    // 解析路径（可能带 ?query）
    const char* question = std::find(start, space, '?');
    if (question != space) {
        request_.setPath(start, question);
        request_.setQuery(question + 1, space);
    } else {
        request_.setPath(start, space);
    }

    // 解析版本
    start = space + 1;
    if (end - start != 8) return false;
    if (std::equal(start, end, "HTTP/1.1")) {
        request_.setVersion(HttpRequest::kHttp11);
    } else if (std::equal(start, end, "HTTP/1.0")) {
        request_.setVersion(HttpRequest::kHttp10);
    } else {
        return false;
    }

    return true;
}