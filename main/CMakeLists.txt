add_executable(vgame "${CMAKE_CURRENT_LIST_DIR}/main.cpp")
target_compile_definitions(vgame PRIVATE VGAME_CLIENT)
target_include_directories(vgame PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(vgame PRIVATE common client)
