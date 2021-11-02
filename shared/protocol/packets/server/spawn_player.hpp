/*
 * spawn_player.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct SpawnPlayer final : public ServerPacket<0x005> {
    uint32_t network_id;
    uint32_t session_id;
    float3::value_type head_angles[2];
    float3::value_type position[3];
    float yaw;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(network_id);
        s.value4b(session_id);
        s.container4b(head_angles);
        s.container4b(position);
        s.value4b(yaw);
    }
};
} // namespace protocol::packets
