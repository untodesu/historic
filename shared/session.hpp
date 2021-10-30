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

enum class SessionState {
    DISCONNECTED,
    CONNECTED,
    LOGGING_IN,
    LOADING_GAMEDATA,
    PLAYING
};

struct Session {
    uint32_t id { 0 };
    ENetPeer *peer { nullptr };
    SessionState state { SessionState::CONNECTED };
    std::string username { "unnamed" };
    entt::entity player_entity { entt::null };
};

struct ClientSession final : public Session {
    // server-side entity ID
    uint32_t player_network_id { 0 };
};
