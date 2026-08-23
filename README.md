# LiteReactor

A lightweight high-performance HTTP server built with C++17.

> 🚧 Work in progress. Deadline: 8/25.

## Current Progress

| Day | Topic | Status |
|-----|-------|--------|
| Day -7 | Blocking TCP Echo Server | ✅ |
| Day -6 | Non-blocking IO with polling | ✅ |
| Day -5 | Epoll-based echo server (multi-client) | ✅ |
| Day -4 | ThreadPool with condition variable | ✅ |
| Day -3 | Buffer (network buffer) | ✅ |
| Day -2 | MemoryPool (fixed block allocator) | ✅ |
| Day -1 | InetAddress + Socket RAII | ✅ |
| Day 0 | Channel + Poller + EventLoop | ✅ |
| Day 1 | Acceptor | ✅ |
| Day 2 | TcpConnection | ✅ |
| Day 3 | TcpServer (echo working) | ✅ |
| Day 4 | HttpParser + HttpResponse + HttpContext | ✅ |
| Day 5 | HttpServer (browser can access static files) | ✅ |

## Build & Run

```bash
mkdir -p build && cd build
cmake ..
make
./test_httpserver
# Open http://localhost:8080/ in browser