add_library(client STATIC "")
target_compile_definitions(client PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(client PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(client PUBLIC core glad glfw imgui stb)
target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/config.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp")
add_subdirectory(gl)

# We don't have "shared" module yet
# FUCKING CASE SENSITIVE NAMES
target_link_libraries(client PUBLIC EnTT enet)
