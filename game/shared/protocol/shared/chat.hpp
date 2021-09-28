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
struct Chat final : public BasePacket<0xF001> {
    std::string message;

    template<typename S>
    inline void serialize(S &s)
    {
        s.text1b(message, 2048);
    }
};
} // namespace protocol

