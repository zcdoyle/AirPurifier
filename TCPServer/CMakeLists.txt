cmake_minimum_required(VERSION 2.8)
project(TCPServer)

set(CMAKE_BUILD_TYPE "release")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "debug")
endif()

set(CXX_FLAGS
        -g
        # -DVALGRIND
        # -DMUDUO_STD_STRING
        # -Wall
        # -Wextra
        # -m32
        # -Werror
        # -Wconversion
        -Wno-unused-parameter
        # -Wold-style-cast
        -Woverloaded-virtual
        -Wpointer-arith
        #-Wshadow
        -Wwrite-strings
        -march=native
        # -MMD
        -std=c++11
        -rdynamic
        )

string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -finline-limit=1000 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

include_directories(${PROJECT_SOURCE_DIR})

# muduo
if(NOT MUDUO_PATH)
    #set(MUDUO_PATH "/home/$ENV{USER}/third_lib/muduo-master/build/${CMAKE_BUILD_TYPE}-install")
   set(MUDUO_PATH "/usr/hao/Third_Party/muduo/${CMAKE_BUILD_TYPE}-install")
endif()
find_path(Muduo_INCLUDE_DIR muduo "${MUDUO_PATH}/include")
find_path(Muduo_LIBRARY_DIR libmuduo_net.a "${MUDUO_PATH}/lib")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} ${Muduo_LIBRARY_DIR})
message(STATUS ${Muduo_INCLUDE_DIR})
message(STATUS ${Muduo_LIBRARY_DIR})
include_directories(${Muduo_INCLUDE_DIR})
find_library(muduo_base muduo_base)
find_library(muduo_net muduo_net)
find_library(muduo_protorpc muduo_protorpc)
find_library(muduo_protobuf_codec muduo_protobuf_codec)
find_library(muduo_protorpc_wire muduo_protorpc_wire)
message(STATUS ${muduo_base})
message(STATUS ${muduo_net})
message(STATUS ${muduo_protorpc})
message(STATUS ${muduo_protobuf_codec})
message(STATUS ${muduo_protorpc_wire})

# protobuf
set(PROTOBUF_PATH "/usr/include/google/protobuf")
include_directories(${PROTOBUF_PATH})
find_library(protobuf protobuf)
include_directories(protobuf)


# boost
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# rapidJson
include_directories(json)
include_directories(json/rapidjson)

set(SOURCE_FILES
        main.cc
        FrameStruct.h
        EEPROMStruct.h
        TCPCodec.h TCPCodec.cpp
        daemonize.cpp daemonize.h
        server.cpp server.h
        configuration.cc configuration.h
        dispatcher.cpp dispatcher.h
        MessageHandler.cpp MessageHandler.h
        MessageConstructor.cpp MessageConstructor.h
        protobuf/AirPurifier.ProtoMessage.pb.cc protobuf/AirPurifier.ProtoMessage.pb.h
        protobuf/ProtobufCodec.cc protobuf/ProtobufCodec.h
        protobuf/google-inl.h
        json/JsonCodec.h
        RpcClient.h
        JsonHandler.h JsonHandler.cc
        http.cc http.h
	aes.c aes.h
	crc.cpp crc.h
    )

add_executable(TCPServer ${SOURCE_FILES})
target_link_libraries(TCPServer ${muduo_protorpc})
target_link_libraries(TCPServer ${muduo_net})
target_link_libraries(TCPServer ${muduo_base})
target_link_libraries(TCPServer ${protobuf})
target_link_libraries(TCPServer ${muduo_protobuf_codec})
target_link_libraries(TCPServer ${muduo_protorpc_wire})
target_link_libraries(TCPServer boost_system)
target_link_libraries(TCPServer ${Boost_LIBRARIES})
target_link_libraries(TCPServer hiredis)
target_link_libraries(TCPServer pthread rt)
# zlib
target_link_libraries(TCPServer z)
# tcmalloc
target_link_libraries(TCPServer tcmalloc)

