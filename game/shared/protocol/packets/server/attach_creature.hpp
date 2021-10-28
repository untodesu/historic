/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct AttachCreature final : public ServerPacket<0x006> {
    uint32_t network_id;
    float3::value_type position[3];
    float yaw;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(network_id);
        s.container4b(position);
        s.value4b(yaw);
    }
};
} // namespace protocol::packets
