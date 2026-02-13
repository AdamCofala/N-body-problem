cmake_minimum_required(VERSION 3.16)

add_library(khr INTERFACE)
target_include_directories(khr INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
