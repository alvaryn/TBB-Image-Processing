cmake_minimum_required(VERSION 3.9)
project(tutorial_15_parallel_for)

set(CMAKE_CXX_STANDARD 11)

add_executable(tutorial_15_parallel_for main.cpp)
target_link_libraries(tutorial_15_parallel_for tbb freeimageplus)