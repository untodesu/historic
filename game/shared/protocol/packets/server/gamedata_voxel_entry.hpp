/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/protocol/protocol.hpp>
#include <game/shared/voxels.hpp>

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
