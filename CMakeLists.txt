cmake_minimum_required(VERSION 3.16)

project(pmlh)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wno-unused-variable")
link_libraries(pmem)
aux_source_directory(./src DIR_SRCS)
add_executable(pmlhash ${DIR_SRCS})