cmake_minimum_required(VERSION 3.5)
project(WTS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mavx -mavx2 -msse3  -fopenmp ")

include_directories(include)



add_executable(HNSW main.cpp)
