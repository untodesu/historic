add_library(client STATIC "")
target_compile_definitions(client PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(client PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(client PUBLIC common glad glfw imgui stb)
target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/chunks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/config.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/deferred_pass.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/game.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gbuffer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/image.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/player_look.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/player_move.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screenshots.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/view.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_mesher.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/voxel_renderer.cpp")
add_subdirectory(gl)
