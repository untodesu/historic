/*
 * request_respawn.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct RequestRespawn final : public ClientPacket<0x004> {
    template<typename S>
    inline void serialize(S &s)
    {
        
    }
};
} // namespace protocol::packets
