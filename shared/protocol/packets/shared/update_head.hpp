/*
 * update_head.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct UpdateHead final : public SharedPacket<0x003> {
    uint32_t entity_id;
    float2::value_type angles[2];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(entity_id);
        s.container4b(angles);
    }
};
} // namespace protocol::packets
