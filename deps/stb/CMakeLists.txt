cmake_minimum_required(VERSION 3.10)
project(stb LANGUAGES C)
add_library(stb STATIC "${CMAKE_CURRENT_LIST_DIR}/stb_impl.c")
target_include_directories(stb PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
