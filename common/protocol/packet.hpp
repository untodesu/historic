/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/adapter/measure_size.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <common/types.hpp>

namespace protocol
{
template<uint16_t binding, uint16_t id>
struct Packet {
    constexpr static const uint16_t HEADER = (binding & 0xF000) | (id & 0x0FFF);
};

template<uint16_t id>
struct ClientboundPacket : Packet<0x1000, id> {};
template<uint16_t id>
struct ServerboundPacket : Packet<0x2000, id> {};
template<uint16_t id>
struct UnboundPacket : Packet<0xF000, id> {};
} // namespace protocol
