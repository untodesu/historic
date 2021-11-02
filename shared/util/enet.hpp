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
static inline void broadcastPacket(ENetHost *host, const T &packet, uint8_t channel, uint32_t flags)
{
    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
    enet_host_broadcast(host, channel, enet_packet_create(pbuf.data(), pbuf.size(), flags | ENET_PACKET_FLAG_RELIABLE));
}

template<typename T>
static inline void sendPacket(ENetPeer *peer, const T &packet, uint8_t channel, uint32_t flags)
{
    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
    enet_peer_send(peer, channel, enet_packet_create(pbuf.data(), pbuf.size(), flags | ENET_PACKET_FLAG_RELIABLE));
}
} // namespace util
