add_library(server STATIC "")
target_include_directories(server PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(server PUBLIC common shared)
target_sources(server PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/server_app.cpp")
