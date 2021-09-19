cmake_minimum_required(VERSION 3.10)
project(voxelius LANGUAGES CXX VERSION 0.0.1)

cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(UVRE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# shared dependencies
add_subdirectory(deps/entt)
add_subdirectory(deps/glm)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/stb)
add_subdirectory(deps/thread_pool)

# client-side dependencies
add_subdirectory(deps/glad)
add_subdirectory(deps/glfw)

set(vgame_target vgame)
add_executable(${vgame_target} "")
target_compile_definitions(${vgame_target} PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(${vgame_target} PRIVATE EnTT glm spdlog stb thread_pool)
add_subdirectory(src)

if(MSVC)
    # C4307 is being thrown EVERY FUCKING TIME the
    # compiler traverses through EnTT headers. Annoying!
    target_compile_options(${vgame_target} PRIVATE /wd4307)
endif()
