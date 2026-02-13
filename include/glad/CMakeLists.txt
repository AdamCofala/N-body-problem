cmake_minimum_required(VERSION 3.16)

add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/glad.c")
target_include_directories(glad PUBLIC
	"${CMAKE_CURRENT_SOURCE_DIR}"
	"${CMAKE_CURRENT_SOURCE_DIR}/.."
)
