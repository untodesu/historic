/*
 * request_gamedata.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct RequestGamedata final : public ClientPacket<0x003> {
    template<typename S>
    inline void serialize(S &s)
    {
        
    }
};
} // namespace protocol::packets
