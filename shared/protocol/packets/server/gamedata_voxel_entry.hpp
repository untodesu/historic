/*
 * gamedata_voxel_entry.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/voxels.hpp>

namespace protocol::packets
{
struct GamedataVoxelEntry final : public ServerPacket<0x001> {
    voxel_t voxel;
    VoxelType type;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value1b(voxel);
        s.value1b(type);
    }
};
} // namespace protocol::packets
