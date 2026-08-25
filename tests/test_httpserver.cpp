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

        // 循环 buffer 里可能存在的多个完整请求
        while (buf->readableBytes() > 0) {
            if (!context->parseRequest(buf)) {
                // 数据不够一个完整请求，或者解析错误
                break;
            }

            if (!context->gotAll()) {
                // 理论上不会到这里，等下次数据
                break;
            }

            const HttpRequest& req = context->request();
            std::cout << "收到请求: " << req.path() << "\n";

            // 构造响应
            std::string filename = "www" + (req.path() == "/" ? "/index.html" : req.path());
            std::string body = readFile(filename);

            // false = Keep-Alive
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

            // 处理完一个请求，重置 HttpContext 准备下一个
            context->reset();
        }
    });

    server.start();
    std::cout << "HttpServer on 8080 (Keep-Alive)\n";
    loop.loop();
    return 0;
}