/*
 * creature_position.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct CreaturePosition final : public SharedPacket<0x001> {
    uint32_t network_id;
    float3::value_type position[3];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(network_id);
        s.container4b(position);
    }
};
} // namespace protocol::packets
