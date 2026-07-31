# LiteReactor

A lightweight high-performance HTTP server built with C++17.

> 🚧 Work in progress.

## Current Progress

| Day | Topic | Status |
|-----|-------|--------|
| Day -7 | Blocking TCP Echo Server | ✅ |
| Day -6 | Non-blocking IO with polling | ✅ |

## Build

```bash
g++ -std=c++17 -Wall echo_server.cpp -o echo_server
./echo_server 8080
