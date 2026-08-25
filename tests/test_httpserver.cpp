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

    // 连接建立/断开时：创建/销毁 HttpContext
    server.setConnectionCallback([](const TcpConnection::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            // 新连接：分配一个 HttpContext
            conn->setContext(new HttpContext());
            std::cout << "UP: " << conn->peerAddress().toIpPort() << "\n";
        } else {
            // 连接断开：释放
            delete static_cast<HttpContext*>(conn->getContext());
            std::cout << "DOWN: " << conn->name() << "\n";
        }
    });

    server.setMessageCallback([](const TcpConnection::TcpConnectionPtr& conn,
                                  Buffer* buf) {
        // 从连接里取出属于它的 HttpContext
        HttpContext* context = static_cast<HttpContext*>(conn->getContext());
        
        if (!context->parseRequest(buf)) {
            return;  // 数据不够，等下次
        }
        
        if (context->gotAll()) {
            const HttpRequest& req = context->request();
            std::cout << "收到请求: " << req.path() << "\n";
            
            // 构造响应
            std::string filename = "www" + (req.path() == "/" ? "/index.html" : req.path());
            std::string body = readFile(filename);
            
            // false = Keep-Alive！
            HttpResponse response(false);
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
            
            // 不 shutdown！保持连接！
            
            // 翻到新的一页，等下一个请求
            context->reset();
        }
    });

    server.start();
    std::cout << "HttpServer on 8080 (Keep-Alive)\n";
    loop.loop();
    return 0;
}