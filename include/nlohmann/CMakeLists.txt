cmake_minimum_required(VERSION 3.16)

add_library(nlohmann INTERFACE)
target_include_directories(nlohmann INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
