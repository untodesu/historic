target_sources(client PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/atlas.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/gbuffer.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/shadowmap.cpp")
add_subdirectory(gl)
