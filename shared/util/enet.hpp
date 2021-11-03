/*
 * enet.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <enet/enet.h>
#include <shared/protocol/protocol.hpp>

namespace util
{
template<typename T>
static inline void broadcastPacket(ENetHost *host, const T &packet, uint8_t channel, uint32_t flags, ENetPeer *sender = nullptr)
{
    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
    ENetPacket *packet_p = enet_packet_create(pbuf.data(), pbuf.size(), flags | ENET_PACKET_FLAG_RELIABLE);

    for(ENetPeer *peer = &host->peers[0]; peer <= &host->peers[host->peerCount]; peer++) {
        if(peer == sender)
            continue;
        enet_peer_send(peer, 0, packet_p);
    }

    if(packet_p->referenceCount == 0)
        enet_packet_destroy(packet_p);
}

template<typename T>
static inline void sendPacket(ENetPeer *peer, const T &packet, uint8_t channel, uint32_t flags)
{
    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
    enet_peer_send(peer, channel, enet_packet_create(pbuf.data(), pbuf.size(), flags | ENET_PACKET_FLAG_RELIABLE));
}
} // namespace util
