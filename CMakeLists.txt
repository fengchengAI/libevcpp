cmake_minimum_required(VERSION 3.16)
project(libevcpp)

set(CMAKE_CXX_STANDARD 14)
file(GLOB source_list "*.cpp" "*.h")
add_executable(libevcpp ${source_list} utils.cpp ev_other_watcher.cpp ev_other_watcher.h ev_stat.cpp ev_stat.h)