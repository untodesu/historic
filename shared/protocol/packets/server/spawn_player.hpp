/*
 * spawn_player.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct SpawnPlayer final : public ServerPacket<0x00A> {
    uint32_t entity_id;
    uint32_t session_id;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(entity_id);
        s.value4b(session_id);
    }
};
} // namespace protocol::packets
