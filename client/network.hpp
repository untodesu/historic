/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <common/math/types.hpp>
#include <string>

namespace cl_network
{
void init();
void shutdown();
bool connect(const std::string &host, uint16_t port);
void disconnect(const std::string &reason);
void update();
entt::entity createEntity(uint32_t network_id);
entt::entity findEntity(uint32_t network_id);
void removeEntity(uint32_t network_id);
} // namespace cl_network

namespace network = cl_network;
