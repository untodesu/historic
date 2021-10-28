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
struct Handshake final : public ClientPacket<0x000> {
    uint16_t version { protocol::VERSION };

    template<typename S>
    inline void serialize(S &s)
    {
        s.value2b(version);
    }
};
} // namespace protocol::packets
