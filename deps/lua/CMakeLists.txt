cmake_minimum_required(VERSION 3.10)
project(lua LANGUAGES C VERSION 5.4.3)
add_library(lua STATIC
    "${CMAKE_CURRENT_LIST_DIR}/lapi.c"
    "${CMAKE_CURRENT_LIST_DIR}/lauxlib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lbaselib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lcode.c"
    "${CMAKE_CURRENT_LIST_DIR}/lcorolib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lctype.c"
    "${CMAKE_CURRENT_LIST_DIR}/ldblib.c"
    "${CMAKE_CURRENT_LIST_DIR}/ldebug.c"
    "${CMAKE_CURRENT_LIST_DIR}/ldo.c"
    "${CMAKE_CURRENT_LIST_DIR}/ldump.c"
    "${CMAKE_CURRENT_LIST_DIR}/lfunc.c"
    "${CMAKE_CURRENT_LIST_DIR}/lgc.c"
    "${CMAKE_CURRENT_LIST_DIR}/linit.c"
    "${CMAKE_CURRENT_LIST_DIR}/liolib.c"
    "${CMAKE_CURRENT_LIST_DIR}/llex.c"
    "${CMAKE_CURRENT_LIST_DIR}/lmathlib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lmem.c"
    "${CMAKE_CURRENT_LIST_DIR}/loadlib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lobject.c"
    "${CMAKE_CURRENT_LIST_DIR}/lopcodes.c"
    "${CMAKE_CURRENT_LIST_DIR}/loslib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lparser.c"
    "${CMAKE_CURRENT_LIST_DIR}/lstate.c"
    "${CMAKE_CURRENT_LIST_DIR}/lstring.c"
    "${CMAKE_CURRENT_LIST_DIR}/lstrlib.c"
    "${CMAKE_CURRENT_LIST_DIR}/ltable.c"
    "${CMAKE_CURRENT_LIST_DIR}/ltablib.c"
    "${CMAKE_CURRENT_LIST_DIR}/ltm.c"
    "${CMAKE_CURRENT_LIST_DIR}/lundump.c"
    "${CMAKE_CURRENT_LIST_DIR}/lutf8lib.c"
    "${CMAKE_CURRENT_LIST_DIR}/lvm.c"
    "${CMAKE_CURRENT_LIST_DIR}/lzio.c")
target_include_directories(lua PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
if(UNIX AND NOT EMSCRIPTEN)
    target_compile_definitions(lua PUBLIC LUA_USE_POSIX)
endif()
