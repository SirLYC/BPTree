cmake_minimum_required(VERSION 3.12)
project(BPTree)

set(CMAKE_CXX_STANDARD 11)

add_executable(BPTree main.cpp BPTree.h List.h Comp.h Test.h Test.cpp)