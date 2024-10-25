cmake_minimum_required(VERSION 3.5)
project(WTS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mavx -mavx2 -msse3  -fopenmp ")

include_directories(include)

###FOR TCMALLOC
set(Gperftools_INCLUDE_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/gperftools/src/)
set(Gperftools_LIBRARY_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/gperftools/build/)
include_directories(${Gperftools_INCLUDE_DIR})
link_directories(${Gperftools_LIBRARY_DIR})


add_executable(WTSS main.cpp)
target_link_libraries(WTSS -ltcmalloc)