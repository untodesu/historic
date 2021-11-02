/*
 * handshake.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct Handshake final : public ClientPacket<0x000> {
    uint16_t version { protocol::VERSION };

    template<typename S>
    inline void serialize(S &s)
    {
        s.value2b(version);
    }
};
} // namespace protocol::packets
