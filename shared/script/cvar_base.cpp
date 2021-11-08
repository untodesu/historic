/*
 * cvar_base.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <shared/script/cvar_base.hpp>
#include <shared/script/cvars.hpp>

CVarBase::CVarBase(const std::string &name, const std::string &default_value, const fcvar_t flags)
    : name(name), flags(flags), value(default_value)
{

}

void CVarBase::set(const std::string &value)
{
    // set() calls may be avoided via CVarSetHack
    // that is used when init.js and user.js are executed.
    if(!cvars::getInitMode() && (flags & FCVAR_READONLY))
        return;
    touch(value);
}

void CVarBase::touch(const std::string &value)
{
    this->value = value;
}
