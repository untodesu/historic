/*
 * disconnect.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <string>

namespace protocol::packets
{
struct Disconnect final : public SharedPacket<0x000> {
    std::string reason { "Disconnected." };

    template<typename S>
    inline void serialize(S &s)
    {
        s.text1b(reason, 255);
    }
};
} // namespace protocol::packets
