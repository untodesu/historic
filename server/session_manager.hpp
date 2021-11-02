/*
 * session_manager.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/session.hpp>

namespace session_manager
{
void init();
Session *create();
Session *find(uint32_t session_id);
void destroy(Session *session);
void kick(Session *session, const std::string &reason);
void kickAll(const std::string &reason);
} // namespace session_manager
