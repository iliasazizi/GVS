set(CMAKE_CXX_STANDARD 11)

#add_executable(test_nsg_index test_nsg_index.cpp)
#target_link_libraries(test_nsg_index ${PROJECT_NAME} -ltcmalloc)

#add_executable(test_nsg_search test_nsg_search.cpp)
#target_link_libraries(test_nsg_search ${PROJECT_NAME})

#add_executable(test_nsg_optimized_search test_nsg_optimized_search.cpp)
#target_link_libraries(test_nsg_optimized_search ${PROJECT_NAME} -ltcmalloc)

add_executable(nsg nsg.cpp)
target_link_libraries(nsg ${PROJECT_NAME} -ltcmalloc  ${Boost_LIBRARIES})

add_executable(kmeans kmeans.cpp)
target_link_libraries(kmeans ${PROJECT_NAME} -ltcmalloc  ${Boost_LIBRARIES})


