/*
 * cvar.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <unordered_map>

using cvar_flags_t = unsigned int;
constexpr static const cvar_flags_t FCVAR_ARCHIVE   = (1 << 0); // Saved to init.js upon shutting down
constexpr static const cvar_flags_t FCVAR_INIT_ONLY = (1 << 1); // Writable only during init.js and user.js

class CVar final {
    friend class CVarList;

public:
    CVar(const std::string &name, const std::string &default_value, cvar_flags_t flags = 0);

    void set(const std::string &value);
    void set(const float value);
    void set(const int value);

    const std::string &getString() const;
    const float getFloat() const;
    const int getInt() const;

    const std::string &getName() const;
    const cvar_flags_t getFlags() const;

private:
    std::string name;
    cvar_flags_t flags;
    std::string v_string;
    float v_float;
    int v_int;
};

enum class CVarDumpMode {
    KEY_VALUE_PAIRS,
    SCRIPT_SOURCE
};

class CVarList final {
public:
    using map_type = std::unordered_map<std::string, CVar *>;
    using iterator = map_type::iterator;
    using const_iterator = map_type::const_iterator;

public:
    void erase(const_iterator it);
    bool insert(CVar &cvar);
    const std::string dump(CVarDumpMode mode);

    inline void maskWrite(cvar_flags_t mask)
    {
        this->mask |= mask;
    }

    inline void unmaskWrite(cvar_flags_t mask)
    {
        this->mask &= ~mask;
    }

    inline iterator find(const std::string &name)
    {
        iterator it = cvars.find(name);
        if(it != cvars.end() && !(it->second->flags & mask))
            return it;
        return cvars.end();
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
    cvar_flags_t mask { FCVAR_INIT_ONLY };
    map_type cvars;
};
