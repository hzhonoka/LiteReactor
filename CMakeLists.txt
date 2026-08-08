cmake_minimum_required(VERSION 3.10)
project(LiteReactor)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -g -pthread")

# 头文件搜索路径
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${PROJECT_SOURCE_DIR}/src/base)

# 测试：线程池
add_executable(test_threadpool tests/test_threadpool.cpp)
target_link_libraries(test_threadpool pthread)

# 测试：epoll echo（保留参考）
add_executable(test_epoll_echo tests/epoll_echo.cpp)
target_link_libraries(test_epoll_echo pthread)

# 测试：Buffer
add_executable(test_buffer tests/test_buffer.cpp src/base/Buffer.cpp)

add_executable(test_memorypool tests/test_memorypool.cpp src/base/MemoryPool.cpp)
