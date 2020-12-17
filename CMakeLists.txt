cmake_minimum_required(VERSION 3.16)
project(libevcpp)

set(CMAKE_CXX_STANDARD 14)
file(GLOB source_list "*.cpp" "*.h")
list(REMOVE_ITEM source_list "/home/feng/Github/CLionProjects/libevcpp/test.cpp")
message(${source_list})
add_executable(libevcpp ${source_list})
