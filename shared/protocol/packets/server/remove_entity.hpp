/*
 * remove_entity.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct RemoveEntity final : public ServerPacket<0x009> {
    uint32_t entity_id;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(entity_id);
    }
};
} // namespace protocol::packets
