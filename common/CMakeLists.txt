add_library(common STATIC "")
target_include_directories(common PUBLIC "${CMAKE_SOURCE_DIR}")
target_link_libraries(common PUBLIC enet EnTT glm spdlog thread_pool toml)

target_sources(common PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/cmdline.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/fs.cpp")
add_subdirectory(math)
add_subdirectory(util)

if(WIN32)
    # Windows.h conflicts with std::numeric_limits<T>
    target_compile_definitions(common PUBLIC NOMINMAX)
endif()

if(MSVC)
    # Lets celebrate and make MSVC to suck some dick
    target_compile_definitions(common PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()
