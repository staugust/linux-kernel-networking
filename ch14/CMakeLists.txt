cmake_minimum_required(VERSION 3.0)
#project name
PROJECT(linux-kernel-networking)

#include_directories(/usr/include/libnl3)



#AUX_SOURCE_DIRECTORY(src DIR_SRCS)
ADD_EXECUTABLE(unsharepid src/pid.cc)
