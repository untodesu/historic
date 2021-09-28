/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <enet/enet.h>
#include <string>

namespace network
{
void init(size_t peers, uint16_t port = 43103);
void shutdown();
void disconnect(ENetPeer *peer, const std::string &reason);
bool event(ENetEvent &event, size_t timeout = 5);
void broadcast(size_t size, const void *data, uint8_t channel = 0, uint32_t flags = ENET_PACKET_FLAG_RELIABLE);
} // namespace network
