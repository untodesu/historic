target_link_libraries(${vgame_target} PRIVATE glfw uvre)
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_globals.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_world.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/res_texture.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/screen.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/uvre_devicewrap.cpp")
add_subdirectory(sys)
