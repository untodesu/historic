/*
 * script_engine.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/globals.hpp>
#include <client/script_engine.hpp>
#include <spdlog/spdlog.h>
#include <sstream>
#include <unordered_map>

static int scriptPrint(lua_State *L)
{
    const int argc = lua_gettop(L);
    std::stringstream ss;
    for(int i = 0; i < argc; i++)
        ss << luaL_tolstring(L, i + 1, nullptr);
    spdlog::info(ss.str());
    return 0;
}

void script_engine::init()
{
    script_engine::shutdown();

    globals::script_runtime = luaL_newstate();
    luaL_openlibs(globals::script_runtime);

    script_engine::addFunc("print", &scriptPrint);
}

void script_engine::shutdown()
{
    if(globals::script_runtime) {
        lua_close(globals::script_runtime);
        globals::script_runtime = nullptr;
    }
}

void script_engine::addFunc(const std::string &name, lua_CFunction func)
{
    lua_pushcfunction(globals::script_runtime, func);
    lua_setglobal(globals::script_runtime, name.c_str());
}

void script_engine::runFile(const stdfs::path &path)
{
    std::string source;
    if(fs::readText(path, source)) {
        script_engine::runString(source);
        return;
    }

    spdlog::warn("Unable to read {}", path.string());
}

void script_engine::runString(const std::string &str)
{
    if(!luaL_dostring(globals::script_runtime, str.c_str()))
        return;
    spdlog::error(luaL_tolstring(globals::script_runtime, -1, nullptr));
}
