/*
 * api.hpp
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

namespace cl_api
{
void init();
void shutdown();
void expose(const std::string &entry, lua_CFunction func);
void run(const std::string &source);
void run(const stdfs::path &path);
} // namespace cl_api

namespace api = cl_api;
