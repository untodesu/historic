/*
 * chunk_voxels.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <shared/protocol/protocol.hpp>
#include <shared/world.hpp>

namespace protocol::packets
{
struct ChunkVoxels final : public ServerPacket<0x005> {
    chunkpos_t::value_type position[3];
    voxel_array_t data;

    template<typename S>
    inline void serialize(S &s)
    {
        s.container4b(position);
        s.container1b(data);
    }
};
} // namespace protocol::packets
