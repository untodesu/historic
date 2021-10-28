/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/protocol/protocol.hpp>

namespace protocol::packets
{
struct CreaturePosition final : public SharedPacket<0x001> {
    uint32_t network_id;
    float3::value_type position[3];

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(network_id);
        s.container4b(position);
    }
};
} // namespace protocol::packets
