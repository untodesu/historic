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
struct CreatureRotation : public SharedPacket<0x002> {
    uint64_t network_id;
    float rotation;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value8b(network_id);
        s.value4b(rotation);
    }
};
} // namespace protocol::packets
