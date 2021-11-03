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
    uint16_t protocol_version { protocol::VERSION };

    template<typename S>
    inline void serialize(S &s)
    {
        s.value2b(protocol_version);
    }
};
} // namespace protocol::packets
