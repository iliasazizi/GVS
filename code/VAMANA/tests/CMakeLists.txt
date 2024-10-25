# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT license.

set(CMAKE_CXX_STANDARD 14)

#add_executable(vamana vamana.cpp ${PROJECT_SOURCE_DIR}/src/aux_utils.cpp )
#if(MSVC)
#	target_link_options(vamana PRIVATE /MACHINE:x64 /DEBUG:FULL)
#	target_link_libraries(vamana debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}/diskann_dll.lib)
#	target_link_libraries(vamana optimized ${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}/diskann_dll.lib)
#else()
	#target_link_libraries(vamana ${PROJECT_NAME} aio -ltcmalloc ${Boost_LIBRARIES})
#endif()


add_executable(vamana vamananob.cpp
		${PROJECT_SOURCE_DIR}/src/aux_utils.cpp )
	target_link_libraries(vamana ${PROJECT_NAME} aio -ltcmalloc ${Boost_LIBRARY_DIR})
