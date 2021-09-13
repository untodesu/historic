target_link_libraries(${vgame_target} PRIVATE glfw uvre)
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/camera_controller.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/client_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/generic_renderer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/input.cpp")
