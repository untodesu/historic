/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <string>
#include <unordered_map>

enum class SessionState {
    CONNECTED,
    LOGGING_IN,
    SENDING_GAME_DATA,
    PLAYING
};

struct Session final {
    uint32_t session_id { 0 };
    ENetPeer *peer { nullptr };
    SessionState state { SessionState::CONNECTED };
    std::string username { "unnamed" };
    entt::entity player { entt::null };
};

namespace session_manager
{
void init();
Session *create();
Session *find(uint32_t session_id);
void destroy(Session *session);
void kick(Session *session, const std::string &reason);
void kickAll(const std::string &reason);
} // namespace session_manager


class SessionManager final {
public:
    using map_type = std::unordered_map<uint32_t, Session>;

public:
    Session *create();
    Session *find(uint32_t session_id);
    void destroy(Session *session);
    void kick(Session *session, const std::string &reason);
    void kickAll(const std::string &reason);

private:
    map_type sessions;
};
