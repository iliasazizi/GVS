cmake_minimum_required(VERSION 2.8)

project(NSG)
include_directories(${PROJECT_SOURCE_DIR}/include)
#OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "no OpenMP supprot")
endif()
find_package(Boost REQUIRED COMPONENTS timer chrono system program_options)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
add_definitions (-std=c++11 -O3 -lboost_timer -lboost_chrono -lboost_system -lboost_program_options -march=native -Wall -DINFO)

add_subdirectory(src)
add_subdirectory(tests)
