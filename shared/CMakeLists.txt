add_library(shared STATIC "")
target_include_directories(shared PUBLIC "${GIT_REPO_ROOT}")
target_link_libraries(shared PUBLIC bitsery duktape EnTT common enet)
target_sources(shared PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/cvar.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/script_engine.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/voxels.cpp")

if(MSVC)
    # C4307 is being thrown EVERY FUCKING TIME the
    # compiler traverses through EnTT headers. Annoying!
    target_compile_options(shared PUBLIC /wd4307)
endif()
