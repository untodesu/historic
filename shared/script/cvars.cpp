/*
 * cvars.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <common/util/format.hpp>
#include <shared/script/cvars.hpp>
#include <unordered_map>
#include <sstream>

static bool init_mode = false;
static std::unordered_map<std::string, CVarBase *> cvars_map;

bool cvars::getInitMode()
{
    return init_mode;
}

void cvars::setInitMode(bool mode)
{
    init_mode = mode;
}

void cvars::insert(CVarBase &cvar)
{
    cvars_map[cvar.getName()] = &cvar;
}

CVarBase *cvars::find(const std::string &name)
{
    auto it = cvars_map.find(name);
    if(it != cvars_map.end())
        return it->second;
    return nullptr;
}

const std::string cvars::dump()
{
    std::stringstream ss;
    for(const auto it : cvars_map) {
        if(!(it.second->getFlags() & FCVAR_ARCHIVE))
            continue;
        ss << util::format("CVar.set(\"%s\", \"%s\");", it.second->getName().c_str(), it.second->get().c_str()) << std::endl;
    }

    return ss.str();
}
