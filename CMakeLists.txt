cmake_minimum_required(VERSION 2.6)

project(control_engine  C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CXX_FLAGS
 -std=c++11
 #-Werror
 -Wall
 -Wextra
 -Wconversion
 -Wno-unused-parameter
 -Wold-style-cast
 -Woverloaded-virtual
 -Wpointer-arith
 -Wshadow
 -Wwrite-strings
 -ffast-math
 -rdynamic
 )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

if(NOT LOG_LEVEL)
    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(LOG_LEVEL 0)
    else()
        set(LOG_LEVEL 1)
    endif()
endif()

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_BINARY_DIR}/config.h"
    )

find_package(Threads REQUIRED)
#find_package(glog REQUIRED)

add_subdirectory(src)
add_subdirectory(samples)
