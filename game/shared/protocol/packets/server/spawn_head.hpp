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
struct SpawnHead final : public ServerPacket<0x006> {
    uint64_t network_id;
    float3::value_type offset[3];
    float2::value_type angles[2];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value8b(network_id);
        s.container4b(offset);
        s.container4b(angles);
    }
};
} // namespace protocol::packets
