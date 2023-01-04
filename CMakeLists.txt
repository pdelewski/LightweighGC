cmake_minimum_required(VERSION 3.24)
project(SafeReference)

add_definitions(-DDEBUG)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(SafeReference
        list.h
        primitives.h
        ptr.h
        resource.h
        sr_driver.cpp)
