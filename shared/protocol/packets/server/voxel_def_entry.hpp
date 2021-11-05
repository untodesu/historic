/*
 * voxel_def_entry.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/voxels.hpp>

namespace protocol::packets
{
struct VoxelDefEntry final : public ServerPacket<0x002> {
    voxel_t voxel;
    voxel_type_t type;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value1b(voxel);
        s.value1b(type);
    }
};
} // namespace protocol::packets
