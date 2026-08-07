# LiteReactor

A lightweight high-performance HTTP server built with C++17.

> 🚧 Work in progress.

## Current Progress

| Day | Topic | Status |
|-----|-------|--------|
| Day -7 | Blocking TCP Echo Server | ✅ |
| Day -6 | Non-blocking IO with polling | ✅ |
| Day -5 | Epoll-based echo server (multi-client) | ✅ |
| Day -4 | ThreadPool with condition variable | ✅ |
| Day -3 | Buffer (network buffer) | ✅ |

## Build

```bash
mkdir -p build && cd build
cmake ..
make
./test_buffer