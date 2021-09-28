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
struct LoginSuccess final : public BasePacket<0x0000> {
    uint32_t id;
    std::string username;

    template<typename S>
    inline void serialize(S &s)
    {
        s.value4b(id);
        s.text1b(username, 40);
    }
};
} // namespace protocol

