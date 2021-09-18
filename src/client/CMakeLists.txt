target_link_libraries(${vgame_target} PRIVATE glfw uvre)
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/clock.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shaderlib.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/util.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/world.cpp")
add_subdirectory(res)
add_subdirectory(sys)
