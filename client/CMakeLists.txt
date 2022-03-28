add_library(client STATIC "")
target_compile_definitions(client PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(client PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(client PUBLIC common glad glfw imgui stb)
target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/config.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp")
add_subdirectory(gl)
