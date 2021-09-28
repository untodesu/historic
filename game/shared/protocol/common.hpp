/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <common/math/types.hpp>

namespace protocol
{
using input_adapter = bitsery::InputBufferAdapter<std::vector<uint8_t>>;
using output_adapter = bitsery::OutputBufferAdapter<std::vector<uint8_t>>;

template<uint16_t packet_id>
struct BasePacket {
    static constexpr const uint16_t ID = packet_id;
};
} // namespace protocol
