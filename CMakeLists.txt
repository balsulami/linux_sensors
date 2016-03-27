cmake_minimum_required(VERSION 3.1)
project(highperformancelinuxsensors)

find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")
project(syscall_sensor)

include_directories(./include)

set(SOURCE_FILES
        lib/format.cc
        lib/gzstream.C
        main.cpp)

# lib
#add_executable(bin/syscall_sensor ${SOURCE_FILES})
add_executable(syscall_sensor ${SOURCE_FILES})
target_link_libraries( syscall_sensor ${ZLIB_LIBRARIES} )

