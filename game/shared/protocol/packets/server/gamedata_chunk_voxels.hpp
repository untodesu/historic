/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/protocol/protocol.hpp>
#include <game/shared/world.hpp>

namespace protocol::packets
{
struct GamedataChunkVoxels final : public ServerPacket<0x003> {
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
