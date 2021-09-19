target_include_directories(${vgame_target} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/filesystem.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
add_subdirectory(client)
add_subdirectory(math)
add_subdirectory(shared)
