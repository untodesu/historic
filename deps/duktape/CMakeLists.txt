cmake_minimum_required(VERSION 3.10)
project(duktape LANGUAGES C VERSION 2.6.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_library(duktape STATIC
    "${CMAKE_CURRENT_LIST_DIR}/duktape.c")
target_include_directories(duktape PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
