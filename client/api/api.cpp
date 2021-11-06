/*
 * api.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <client/api/api.hpp>
#include <client/globals.hpp>
#include <shared/cvar.hpp>
#include <spdlog/spdlog.h>

static int apiPrint(lua_State *L)
{
    const int argc = lua_gettop(L);
    std::stringstream ss;
    for(int i = 0; i < argc; i++)
        ss << luaL_tolstring(L, i + 1, nullptr);
    spdlog::info(ss.str());
    return 0;
}

static int apiCVarSet(lua_State *L)
{
    const int argc = lua_gettop(L);
    if(argc != 2)
        return luaL_error(L, "argument mismatch (required: 2, received: %d)", argc);
    
    const std::string cvar_name = luaL_tolstring(L, 1, nullptr);
    CVarList::iterator it = globals::cvars.find(cvar_name);
    if(it == globals::cvars.end())
        return luaL_error(L, "missing cvar: %s", cvar_name.c_str());

    it->second->set(luaL_tolstring(L, 2, nullptr));
    return 0;
}

static int apiCVarGet(lua_State *L)
{
    const int argc = lua_gettop(L);
    if(argc != 1)
        return luaL_error(L, "argument mismatch (required: 1, received: %d)", argc);
    
    CVarList::const_iterator it = globals::cvars.find(luaL_tolstring(L, 1, nullptr));
    if(it == globals::cvars.cend()) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushstring(L, it->second->getString().c_str());
    return 1;
}


void cl_api::init()
{
    cl_api::shutdown();

    globals::lua = luaL_newstate();
    luaL_openlibs(globals::lua);

    cl_api::expose("print", &apiPrint);
    cl_api::expose("cvar_get", &apiCVarGet);
    cl_api::expose("cvar_set", &apiCVarSet);
}

void cl_api::shutdown()
{
    if(globals::lua) {
        lua_close(globals::lua);
        globals::lua = nullptr;
    }
}

void cl_api::expose(const std::string &entry, lua_CFunction func)
{
    lua_pushcfunction(globals::lua, func);
    lua_setglobal(globals::lua, entry.c_str());
}

void cl_api::run(const std::string &source)
{
    if(!luaL_dostring(globals::lua, source.c_str()))
        return;
    spdlog::error(luaL_tolstring(globals::lua, -1, nullptr));
}

void cl_api::run(const stdfs::path &path)
{
    std::string source;
    if(fs::readText(path, source)) {
        cl_api::run(source);
        return;
    }

    spdlog::error("Unable to read {}", path.string());
}
