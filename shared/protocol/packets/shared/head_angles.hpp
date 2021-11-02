/*
 * head_angles.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct HeadAngles final : public SharedPacket<0x002> {
    uint32_t network_id;
    float2::value_type angles[2];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(network_id);
        s.container4b(angles);
    }
};
} // namespace protocol::packets
