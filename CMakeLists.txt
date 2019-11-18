cmake_minimum_required(VERSION 3.0)
#project name
PROJECT(linux-kernel-networking)

set(CMAKE_CXX_STANDARD 11)
include_directories(/usr/src/kernels/$(CMAKE_HOST_SYSTEM_VERSION)/include)

add_subdirectory(ch2)
add_subdirectory(ch3)
add_subdirectory(ch4)
add_subdirectory(ch14)
