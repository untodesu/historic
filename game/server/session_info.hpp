/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <game/shared/protocol/protocol.hpp>
#include <string>

struct SessionInfo final {
    uint32_t session_id;
    std::string username;
    entt::entity entity;
    protocol::SessionState state;
};
