#pragma once
#include <string>
#include <map>

class HttpRequest {
public:
    enum Method { kInvalid, kGet, kPost, kHead };
    enum Version { kUnknown, kHttp10, kHttp11 };

    HttpRequest() : method_(kInvalid), version_(kUnknown) {}

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    bool setMethod(const char* start, const char* end);
    Method method() const { return method_; }

    void setPath(const char* start, const char* end) { path_.assign(start, end); }
    const std::string& path() const { return path_; }

    void setQuery(const char* start, const char* end) { query_.assign(start, end); }
    const std::string& query() const { return query_; }

    void addHeader(const char* start, const char* colon, const char* end);
    std::string getHeader(const std::string& field) const;

    void swap(HttpRequest& that);

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    std::map<std::string, std::string> headers_;
};

inline bool HttpRequest::setMethod(const char* start, const char* end) {
    std::string m(start, end);
    if (m == "GET") method_ = kGet;
    else if (m == "POST") method_ = kPost;
    else if (m == "HEAD") method_ = kHead;
    else method_ = kInvalid;
    return method_ != kInvalid;
}

inline void HttpRequest::addHeader(const char* start, const char* colon, const char* end) {
    std::string field(start, colon);
    ++colon;
    while (colon < end && isspace(*colon)) ++colon;
    std::string value(colon, end);
    headers_[field] = value;
}

inline std::string HttpRequest::getHeader(const std::string& field) const {
    auto it = headers_.find(field);
    return it != headers_.end() ? it->second : "";
}

inline void HttpRequest::swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    headers_.swap(that.headers_);
}