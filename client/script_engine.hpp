/*
 * script_engine.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

namespace script_engine
{
void init();
void shutdown();
void addFunc(const std::string &name, lua_CFunction func);
void runFile(const stdfs::path &path);
void runString(const std::string &str);
} // namespace script_engine
