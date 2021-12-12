/*
 * network.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/session.hpp>

namespace sv_network
{
void init();
void shutdown();
void update();
Session *createSession();
Session *findSession(uint32_t session_id);
void destroySession(Session *session);
void kick(Session *session, const std::string &reason);
void kickAll(const std::string &reason);
} // namespace sv_network

namespace network = sv_network;
