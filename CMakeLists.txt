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

# File structure of the repo is organized
# in a such way that a single project can
# include any header from any other project.
set(GIT_REPO_ROOT "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(deps/bitsery)
add_subdirectory(deps/duktape)
add_subdirectory(deps/enet)
add_subdirectory(deps/entt)
add_subdirectory(deps/glad)
add_subdirectory(deps/glfw)
add_subdirectory(deps/glm)
add_subdirectory(deps/imgui)
add_subdirectory(deps/spdlog)
add_subdirectory(deps/stb)
add_subdirectory(deps/thread_pool)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
add_subdirectory(shared)
add_subdirectory(launch)
