cmake_minimum_required(VERSION 3.16)

project(pmlh)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall -Wno-unused-variable")
link_libraries(pmem)
add_executable(pmlhash main.cc pml_hash.cc)