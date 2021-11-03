/*
 * chat_message.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <string>

namespace protocol::packets
{
struct ChatMessage final : public SharedPacket<0x001> {
    std::string message;

    template<typename S>
    inline void serialize(S &s)
    {
        s.text1b(message, 2047);
    }
};
} // namespace protocol::packets
