/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/protocol/common.hpp>
#include <string>

namespace protocol
{
struct Disconnect final : public BasePacket<0xF000> {
    std::string reason;

    inline Disconnect()
        : reason("Disconnected by User.")
    {

    }

    inline Disconnect(const std::string &reason)
        : reason(reason)
    {
        
    }

    template<typename S>
    inline void serialize(S &s)
    {
        s.text1b(reason, 256);
    }
};
} // namespace protocol
