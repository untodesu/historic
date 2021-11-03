/*
 * session_manager.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/session.hpp>
#include <unordered_map>

namespace session_manager
{
using map_type = std::unordered_map<uint32_t, Session>;
void init();
map_type &all();
Session *create();
Session *find(uint32_t session_id);
void destroy(Session *session);
void kick(Session *session, const std::string &reason);
void kickAll(const std::string &reason);
} // namespace session_manager
