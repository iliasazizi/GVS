
cmake_minimum_required(VERSION 3.10)

project(LSH_APG_Project VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -mavx -mavx2 -msse3  -fopenmp ")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_executable(LSH_APG
        src/main.cpp
        src/e2lsh.cpp
        src/Preprocess.cpp
        src/Query.cpp
        src/GenericTool.cpp
        src/basis.cpp
        src/distances_simd_avx512.cpp
)

target_include_directories(LSH_APG PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

# Link additional libraries (if needed)
# target_link_libraries(LSH_APG PRIVATE some_library)
