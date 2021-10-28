/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <algorithm>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/adapter/measure_size.h>
#include <bitsery/traits/array.h>
#include <bitsery/traits/string.h>
#include <bitsery/traits/vector.h>
#include <common/math/types.hpp>
#include <enet/enet.h>

namespace protocol
{
constexpr static const uint16_t VERSION = 0x0001;
constexpr static const uint16_t DEFAULT_PORT = 43103;

template<uint16_t packet_id>
struct Packet { constexpr static const uint16_t id = packet_id; };
template<uint16_t packet_id>
struct ClientPacket : public Packet<(packet_id & 0x0FFF) | 0x1000> {};
template<uint16_t packet_id>
struct ServerPacket : public Packet<(packet_id & 0x0FFF) | 0x4000> {};
template<uint16_t packet_id>
struct SharedPacket : public Packet<(packet_id & 0x0FFF) | 0xF000> {};

template<typename T>
static inline const std::vector<uint8_t> serialize(const T &data)
{
    std::vector<uint8_t> result;

    // Packet id
    const uint16_t id = ENET_HOST_TO_NET_16(T::id);
    std::copy(reinterpret_cast<const uint8_t *>(&id), reinterpret_cast<const uint8_t *>(&id + 1), std::back_inserter(result));

    // Payload
    std::vector<uint8_t> pbuf;
    size_t size = bitsery::quickSerialization(bitsery::OutputBufferAdapter<std::vector<uint8_t>> { pbuf }, data);
    std::copy(pbuf.cbegin(), pbuf.cbegin() + size, std::back_inserter(result));

    return result;
}

static inline const bool split(const std::vector<uint8_t> &packet, uint16_t &type, std::vector<uint8_t> &payload)
{
    type = 0xFFFF;
    payload.clear();
    if(packet.size() >= sizeof(uint16_t)) {
        std::copy(packet.cbegin(), packet.cbegin() + sizeof(uint16_t), reinterpret_cast<uint8_t *>(&type));
        type = ENET_NET_TO_HOST_16(type);
        if(packet.size() > sizeof(uint16_t))
            std::copy(packet.cbegin() + sizeof(uint16_t), packet.cend(), std::back_inserter(payload));
        return true;
    }

    return false;
}

template<typename T>
static inline const bool deserialize(const std::vector<uint8_t> &payload, T &data)
{
    const auto state = bitsery::quickDeserialization(bitsery::InputBufferAdapter<std::vector<uint8_t>> { payload.cbegin(), payload.size() }, data);
    return (state.first == bitsery::ReaderError::NoError) && state.second;
}
} // namespace protocol
