cmake_minimum_required(VERSION 3.6)
project(TBB_Demo_Reduction)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(TBB_Demo_Reduction ${SOURCE_FILES})
target_link_libraries(TBB_Demo_Reduction tbb freeimageplus)