/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <game/shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct ClientSpawn final : public ServerPacket<0x004> {
    float3 position;
    float3 head_angles;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(position.x);
        s.value4b(position.y);
        s.value4b(position.z);
        s.value4b(head_angles.x);
        s.value4b(head_angles.y);
        s.value4b(head_angles.z);
    }
};
} // namespace protocol::packets
