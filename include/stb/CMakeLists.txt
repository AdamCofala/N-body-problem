cmake_minimum_required(VERSION 3.16)

add_library(stb_image INTERFACE)
add_library(stb_truetype INTERFACE)

target_include_directories(stb_image INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(stb_truetype INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
