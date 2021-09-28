add_library(common STATIC "")
target_include_directories(common PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(common PUBLIC glm spdlog thread_pool)
target_sources(common PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/filesystem.cpp")
add_subdirectory(math)

if(WIN32)
    # Windows.h conflicts with std::numeric_limits<T>
    target_compile_definitions(common PUBLIC NOMINMAX)
endif()
