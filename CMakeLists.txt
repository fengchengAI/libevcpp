cmake_minimum_required(VERSION 3.16)
project(libevcpp)

set(CMAKE_CXX_STANDARD 14)
file(GLOB source_list "*.cpp" "*.h")
add_executable(libevcpp ${source_list})
