cmake_minimum_required(VERSION 3.10)
project(hw_02 CXX)
set(CMAKE_CXX_STANDARD 17)

set(DEBUG True)
#set(DEBUG False)

if(DEBUG)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Wextra -pedantic")
else()	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
endif()

include_directories(include/)

add_library(huffman SHARED
	include/bitio.h src/bitio.cpp
        include/hufftree.h src/hufftree.cpp
        include/huffman_util.h
        include/huffman_archiver.h src/huffman_archiver.cpp
        include/huffman_dearchiver.h src/huffman_dearchiver.cpp
        include/huffman.h
        include/arg_utils.h src/arg_utils.cpp
)

add_executable(hw_02 
	src/main.cpp
)
target_link_libraries(hw_02 huffman)

set(BUILD_TESTING True)
#set(BUILD_TESTING False)

if(BUILD_TESTING)
	include_directories(test/test_include)

	add_executable(test_hw_02
		test/test_src/test.cpp
		test/test_include/doctest.h
	)
	target_link_libraries(test_hw_02 huffman)
endif()
