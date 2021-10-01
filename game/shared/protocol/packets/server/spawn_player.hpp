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
struct SpawnPlayer final : public ServerPacket<0x006> {
    uint32_t session_id;
    float position[3];
    float head_angles[2];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(session_id);
        s.container4b(position);
        s.container4b(head_angles);
    }
};
} // namespace protocol::packets
