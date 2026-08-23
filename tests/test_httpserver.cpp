// tests/test_httpserver.cpp
#include "net/TcpServer.h"
#include "net/EventLoop.h"
#include "net/InetAddress.h"
#include "http/HttpContext.h"
#include "http/HttpResponse.h"
#include "base/Buffer.h"
#include <iostream>
#include <fstream>
#include <sstream>

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

int main() {
    EventLoop loop;
    InetAddress listenAddr(8080);
    TcpServer server(&loop, listenAddr, "LiteReactor");

    server.setMessageCallback([](const TcpConnection::TcpConnectionPtr& conn,
                                  Buffer* buf) {
        // 每个连接一个 HttpContext（需要存在 TcpConnection 里，简化版用静态/全局）
        // 实际应该用 conn->getContext()，今天简化
        static thread_local HttpContext context;  // 每个线程一个，简化
        
        if (!context.parseRequest(buf)) {
            return;  // 数据不够或出错
        }
        
        if (context.gotAll()) {
            const HttpRequest& req = context.request();
            std::cout << "收到请求: " << req.method() << " " << req.path() << "\n";
            
            // 构造响应
            std::string filename = "www" + (req.path() == "/" ? "/index.html" : req.path());
            std::string body = readFile(filename);
            
            HttpResponse response;
            if (!body.empty()) {
                response.setStatusCode(HttpResponse::k200Ok);
                response.setStatusMessage("OK");
                response.setContentType("text/html");
                response.setBody(body);
            } else {
                response.setStatusCode(HttpResponse::k404NotFound);
                response.setStatusMessage("Not Found");
                response.setBody("<h1>404 Not Found</h1>");
            }
            
            Buffer output;
            response.appendToBuffer(&output);
            conn->send(output.retrieveAllAsString());
            
            // 翻到新的一页，支持 Keep-Alive
            context.reset();
        }
    });

    server.start();
    std::cout << "HttpServer on 8080, open http://localhost:8080/\n";
    loop.loop();
    return 0;
}