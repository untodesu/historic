target_link_libraries(${vgame_target} PRIVATE glad glfw)
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_chunks.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp")
add_subdirectory(gl)
add_subdirectory(sys)
add_subdirectory(util)
