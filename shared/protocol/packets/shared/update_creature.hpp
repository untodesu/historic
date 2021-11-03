/*
 * update_creature.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct UpdateCreature final : public SharedPacket<0x002> {
    uint32_t entity_id;
    float3::value_type position[3];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(entity_id);
        s.container4b(position);
    }
};
} // namespace protocol::packets
