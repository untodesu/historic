add_library(core STATIC "")
target_include_directories(core PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(core PUBLIC glm spdlog thread_pool toml)

target_sources(core PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/fs.cpp")
add_subdirectory(math)
add_subdirectory(util)

if(WIN32)
    # Windows.h conflicts with std::numeric_limits<T>
    target_compile_definitions(core PUBLIC NOMINMAX)
endif()

if(MSVC)
    # MSVC pee pee poo poo C functions are dangerous
    target_compile_definitions(core PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
