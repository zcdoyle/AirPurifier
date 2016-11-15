cmake_minimum_required(VERSION 2.8)
project(TCPClient)

set(CMAKE_BUILD_TYPE "release")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "debug")
endif()

set(CXX_FLAGS
        -g
        # -DVALGRIND
        # -DMUDUO_STD_STRING
        -Wall
        -Wextra
        # -m32
        # -Werror
        # -Wconversion
        -Wno-unused-parameter
        # -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        -Wshadow
        -Wwrite-strings
        -march=native
        # -MMD
        # -std=c++0x
        -rdynamic
        )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# muduo
if(NOT MUDUO_PATH)
    set(MUDUO_PATH "/home/$ENV{USER}/third_lib/muduo-master/build/${CMAKE_BUILD_TYPE}-install")
endif()
find_path(Muduo_INCLUDE_DIR muduo "${MUDUO_PATH}/include")
find_path(Muduo_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})
message(STATUS ${Muduo_INCLUDE_DIR})
message(STATUS ${Muduo_LIBRARY_DIR})
include_directories(${Muduo_INCLUDE_DIR})
find_library(muduo_base muduo_base)
find_library(muduo_net muduo_net)
message(STATUS ${muduo_base})
message(STATUS ${muduo_net})

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

include_directories(${PROJECT_SOURCE_DIR})

set(SOURCE_FILES
        codec.h
        configuration.h
        main.cc
        daemonize.h
        daemonize.cpp
        timerClient.h
	aes.h
	aes.c
        )

add_executable(TCPClient ${SOURCE_FILES})
target_link_libraries(TCPClient ${muduo_net})
target_link_libraries(TCPClient ${muduo_base})
target_link_libraries(TCPClient pthread rt)
target_link_libraries(TCPClient tcmalloc)
