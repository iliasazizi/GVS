cmake_minimum_required(VERSION 2.8)

project(LID)
#OpenMP
find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
else()
    message(FATAL_ERROR "no OpenMP supprot")
endif()
set(Boost_INCLUDE_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/boost_1_50_0)
set(Boost_LIBRARY_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/boost_1_50_0/stage/lib/)
find_package(Boost COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIR})
find_package(Boost REQUIRED COMPONENTS timer chrono system program_options)

###FOR TCMALLOC
set(Gperftools_INCLUDE_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/gperftools/src/)
set(Gperftools_LIBRARY_DIR /home/ilias.azizi/lustre/scalableml-um6p-st-sccs-10v5rwpbsmu/azizi-lustre/projects/libs/gperftools/build/)
include_directories(${Gperftools_INCLUDE_DIR})
link_directories(${Gperftools_LIBRARY_DIR})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
add_definitions (-std=c++11 -O3 -lboost_timer -lboost_chrono -lboost_system -lboost_program_options -march=native -Wall -DINFO)


add_executable(dataeval lid.cpp)
target_link_libraries(dataeval ${Boost_LIBRARIES} m)