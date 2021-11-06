/*
 * cvar.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/util/format.hpp>
#include <cstdlib>
#include <unordered_map>
#include <spdlog/spdlog.h>
#include <shared/cvar.hpp>

CVar::CVar(const std::string &name, const std::string &default_value)
    : name(name)
{
    set(default_value);
}

void CVar::set(const std::string &value)
{
    v_string = value;
    v_float = strtof(value.c_str(), nullptr);
    v_int = atoi(value.c_str());
}

void CVar::set(const float value)
{
    v_string = util::format("%f", value);
    v_float = value;
    v_int = static_cast<int>(value);
}

void CVar::set(const int value)
{
    v_string = util::format("%d", value);
    v_float = static_cast<float>(value);
    v_int = value;
}

const std::string &CVar::getString() const
{
    return v_string;
}

const float CVar::getFloat() const
{
    return v_float;
}

const int CVar::getInt() const
{
    return v_int;
}

const std::string &CVar::getName() const
{
    return name;
}

void CVarList::erase(CVarList::const_iterator it)
{
    if(it == cvars.cend())
        return;
    cvars.erase(it);
}

bool CVarList::insert(CVar &cvar)
{
    const auto it = cvars.find(cvar.getName());
    if(it != cvars.cend())
        return false;
    cvars[cvar.getName()] = &cvar;
    return true;
}
