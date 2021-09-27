target_link_libraries(${vgame_target} PRIVATE glad glfw imgui)
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/chunks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/composite.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/debug_overlay.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gbuffer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadow_manager.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadowmap.cpp")
add_subdirectory(gl)
add_subdirectory(sys)
add_subdirectory(util)
