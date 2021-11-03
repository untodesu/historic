/*
 * login_success.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <string>

namespace protocol::packets
{
struct LoginSuccess final : public ServerPacket<0x001> {
    uint32_t session_id;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(session_id);
    }
};
} // namespace protocol::packets
