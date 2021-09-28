/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <common/math/types.hpp>
#include <game/shared/enet/packet.hpp>

namespace protocol
{
using buffer_type = std::vector<uint8_t>;
using input_adapter = bitsery::InputBufferAdapter<buffer_type>;
using output_adapter = bitsery::OutputBufferAdapter<buffer_type>;

template<uint8_t ID>
struct BasePacket {
    static constexpr const uint8_t TYPE = ID;
};

template<typename T>
static inline enet::Packet create(const T &data)
{
    buffer_type buffer;
    size_t size = bitsery::quickSerialization(output_adapter { buffer }, data);
    if(size) {
        enet::Packet packet(size + 1, ENET_PACKET_FLAG_RELIABLE, nullptr);
        const uint8_t type = T::TYPE;
        if(packet.write(0, 1, &type) && packet.write(1, size, buffer.data()))
            return std::move(packet);
    }

    return std::move(enet::Packet(nullptr));
}

static inline bool read(const enet::PacketRef &packet, uint8_t &type, buffer_type &buffer)
{
    if(!packet.read(0, 1, &type))
        return false;

    const size_t size = packet.size() - 1;
    if(!size)
        return false;

    buffer.resize(size);
    if(!packet.read(1, size, buffer.data()))
        return false;

    return true;
}

template<typename T>
static inline bool decode(const buffer_type &buffer, T &data)
{
    const auto state = bitsery::quickDeserialization(input_adapter { buffer.cbegin(), buffer.size() }, data);
    return (state.first == bitsery::ReaderError::NoError) && state.second;
}
} // namespace protocol
