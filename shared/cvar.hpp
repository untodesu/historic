/*
 * cvar.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <string>
#include <unordered_map>

class CVar final {
public:
    CVar(const std::string &name, const std::string &default_value);

    void set(const std::string &value);
    void set(const float value);
    void set(const int value);

    const std::string &getString() const;
    const float getFloat() const;
    const int getInt() const;

    const std::string &getName() const;

private:
    std::string name;
    std::string v_string;
    float v_float;
    int v_int;
};

class CVarList final {
public:
    using map_type = std::unordered_map<std::string, CVar *>;
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

public:
    void erase(const_iterator it);
    bool insert(CVar &cvar);

    inline iterator find(const std::string &name)
    {
        return cvars.find(name);
    }

    inline const_iterator find(const std::string &name) const
    {
        return cvars.find(name);
    }

    inline void clear()
    {
        cvars.clear();
    }

    inline iterator begin()
    {
        return cvars.begin();
    }

    inline iterator end()
    {
        return cvars.end();
    }

    inline const_iterator cbegin() const
    {
        return cvars.cbegin();
    }

    inline const_iterator cend() const
    {
        return cvars.cend();
    }

private:
    map_type cvars;
};
