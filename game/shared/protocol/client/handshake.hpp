/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/protocol/common.hpp>

namespace protocol
{
struct Handshake final : public BasePacket<0x0000> {
    uint16_t version;

    inline Handshake()
        : version(0)
    {

    }

    inline Handshake(uint16_t version)
        : version(version)
    {
        
    }

    template<typename S>
    inline void serialize(S &s)
    {
        s.value2b(version);
    }
};
} // namespace protocol
