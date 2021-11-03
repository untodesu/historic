/*
 * voxel_def_checksum.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct VoxelDefChecksum final : public ServerPacket<0x004> {
    uint64_t checksum;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value8b(checksum);
    }
};
} // namespace protocol::packets
