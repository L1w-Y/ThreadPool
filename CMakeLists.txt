cmake_minimum_required(VERSION 3.30)
project(ThreadPool)

set(CMAKE_CXX_STANDARD 20)

add_executable(ThreadPool main.cpp
                ThreadPool.cpp
                ThreadPool.h)