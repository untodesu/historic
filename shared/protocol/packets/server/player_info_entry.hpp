/*
 * player_info_entry.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct PlayerInfoEntry final : public ServerPacket<0x006> {
    uint32_t session_id;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(session_id);
    }
};
} // namespace protocol::packets
