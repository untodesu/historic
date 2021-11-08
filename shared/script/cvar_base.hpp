/*
 * cvar_base.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/script/fcvar.hpp>
#include <string>

class CVarBase {
public:
    CVarBase(const std::string &name, const std::string &default_value, const fcvar_t flags = 0);
    void set(const std::string &value);

    inline constexpr const std::string &getName() const
    {
        return name;
    }

    inline constexpr fcvar_t getFlags() const
    {
        return flags;
    }

    inline constexpr const std::string &get() const
    {
        return value;
    }

protected:
    virtual void touch(const std::string &value);

private:
    const std::string name;
    const fcvar_t flags;
    std::string value;
};
