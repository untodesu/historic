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
struct ClientSpawn final : public ServerPacket<0x005> {
    float position[3];
    float head_angles[3];

    template<typename S>
    inline void serialize(S &s)
    {
        s.container4b(position);
        s.container4b(head_angles);
    }
};
} // namespace protocol::packets
