/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/protocol/serialization.hpp>

namespace protocol
{
template<typename packet_type>
static inline void broadcast(ENetHost *host, const packet_type &packet, uint32_t flags = 0, ENetPeer *sender = nullptr)
{
    const PacketBuffer buffer = protocol::serialize(packet);
    ENetPacket *net_packet = enet_packet_create(buffer.data(), buffer.size(), flags | ENET_PACKET_FLAG_RELIABLE);

    // Send the packet to all
    // the peers but the sender.
    for(size_t i = 0; i < host->peerCount; i++) {
        if(host->peers[i] == sender)
            continue;
        enet_peer_send(host->peers[i], channel, net_packet);
    }

    if(!net_packet->referenceCount) {
        // Remove the packet - it's not needed anymore.
        enet_packet_destroy(net_packet);
    }
}

template<typename packet_type>
static inline void send(ENetPeer *peer, const packet_type &packet, uint8_t channel, uint32_t flags = 0)
{
    const PacketBuffer buffer = protocol::serialize(packet);
    enet_peer_send(peer, channel, enet_packet_create(buffer.data(), buffer.size(), flags | ENET_PACKET_FLAG_RELIABLE));
}
} // namespace protocol
