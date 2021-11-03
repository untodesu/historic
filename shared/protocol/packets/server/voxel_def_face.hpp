/*
 * voxel_def_face.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/voxels.hpp>

namespace protocol::packets
{
struct VoxelDefFace final : public ServerPacket<0x003> {
    constexpr static const uint8_t TRANSPARENT_BIT = (1 << 0);
    voxel_t voxel;
    VoxelFace face;
    uint8_t flags;
    std::string texture;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value1b(voxel);
        s.value1b(face);
        s.value1b(flags);
        s.text1b(texture, 255);
    }
};
} // namespace protocol::packets
