target_include_directories(${vgame_target} PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
target_sources(${vgame_target} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
add_subdirectory(client)
