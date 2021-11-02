/*
 * login_start.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <string>

namespace protocol::packets
{
struct LoginStart final : public ClientPacket<0x001> {
    std::string username;

    template<typename S>
    inline void serialize(S &s)
    {
        s.text1b(username, 39);
    }
};
} // namespace protocol::packets
