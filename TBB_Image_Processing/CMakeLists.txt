cmake_minimum_required(VERSION 3.15)
project(TBB_Image_Processing)

set(CMAKE_CXX_STANDARD 14)

add_executable(TBB_Image_Processing main.cpp)

target_link_libraries(TBB_Image_Processing tbb pthread freeimageplus)