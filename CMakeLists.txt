cmake_minimum_required(VERSION 3.15)
project(CPPTimer)

set(CMAKE_CXX_STANDARD 14)

add_executable(CPPTimer main.cpp include/CPPTimer.h)
target_include_directories(CPPTimer PUBLIC include)
target_link_libraries(CPPTimer PUBLIC rt)