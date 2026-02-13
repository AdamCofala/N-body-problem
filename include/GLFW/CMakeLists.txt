cmake_minimum_required(VERSION 3.16)

# Header-only shim (this repo contains only GLFW headers)
add_library(glfw_headers INTERFACE)
target_include_directories(glfw_headers INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
