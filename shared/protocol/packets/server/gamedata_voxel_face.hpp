/*
 * gamedata_voxel_face.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/voxels.hpp>

namespace protocol::packets
{
struct GamedataVoxelFace final : public ServerPacket<0x002> {
    voxel_t voxel;
    VoxelFace face;
    uint8_t transparent;
    std::string texture;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value1b(voxel);
        s.value1b(face);
        s.value1b(transparent);
        s.text1b(texture, 255);
    }
};
} // namespace protocol::packets
