cmake_minimum_required(VERSION 3.5)
project(hnsw)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mavx -mavx2 -msse3  -fopenmp ")

include_directories(include)

set(LIBSAX src/sax.cpp)
add_library(libsax STATIC ${LIBSAX})


# Find the HDF5 package
find_package(HDF5 REQUIRED COMPONENTS CXX)

include_directories(${HDF5_INCLUDE_DIRS})

add_executable(HNSW main.cpp)
target_link_libraries(HNSW libsax  ${HDF5_CXX_LIBRARIES})