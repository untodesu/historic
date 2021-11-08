/*
 * cvars.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/script/cvar_numeric.hpp>

namespace cvars
{
bool getInitMode();
void setInitMode(bool mode);
void insert(CVarBase &cvar);
CVarBase *find(const std::string &name);
const std::string dump();
} // namespace cvars
