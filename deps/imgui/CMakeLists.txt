cmake_minimum_required(VERSION 3.10)
project(imgui LANGUAGES CXX VERSION 1.84.2)
add_library(imgui INTERFACE)
target_sources(imgui INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui_impl_glfw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui_stdlib.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui_widgets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui.cpp")
target_include_directories(imgui INTERFACE "${CMAKE_CURRENT_LIST_DIR}")
