add_library(client STATIC "")
target_compile_definitions(client PUBLIC GLFW_INCLUDE_NONE)
target_include_directories(client PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(client PUBLIC common shared glad glfw imgui lua stb)
target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/chunks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/debug_overlay.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/deferred_pass.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/fontlib.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/game.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/network.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadow_manager.cpp")
add_subdirectory(api)
add_subdirectory(render)
add_subdirectory(systems)
add_subdirectory(util)
