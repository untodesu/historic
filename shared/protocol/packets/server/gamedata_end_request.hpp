/*
 * gamedata_end_request.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct GamedataEndRequest final : public ServerPacket<0x004> {
    uint64_t voxel_checksum;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value8b(voxel_checksum);
    }
};
} // namespace protocol::packets
