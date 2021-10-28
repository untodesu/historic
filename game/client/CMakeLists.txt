add_library(client STATIC "")
target_compile_definitions(client PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(client PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(client PUBLIC common shared glad glfw imgui stb)
target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/chunks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/debug_overlay.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/deferred_pass.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/game.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gamedata.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gbuffer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/network_entities.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadow_manager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadowmap.cpp")
add_subdirectory(gl)
add_subdirectory(sys)
add_subdirectory(util)
