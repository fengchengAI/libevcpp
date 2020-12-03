cmake_minimum_required(VERSION 3.16)
project(libevcpp)

set(CMAKE_CXX_STANDARD 14)

add_executable(libevcpp main.cpp watcher.h ev_loop.h ev_epoll.h ev_epoll.cpp ev_loop.cpp watcher.cpp ev_io.cpp ev_io.h)