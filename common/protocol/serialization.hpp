/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <algorithm>
#include <common/protocol/packet.hpp>
#include <enet/enet.h>
#include <iterator>
#include <vector>

namespace protocol
{
using PacketBuffer = std::vector<uint8_t>;
using PayloadBuffer = std::vector<uint8_t>;

template<typename packet_type>
static inline bool combine(const PayloadBuffer &payload, packet_type &packet)
{
    auto adapter = bitsery::InputBufferAdapter<PayloadBuffer>(payload.cbegin(), payload.size());
    const auto state = bitsery::quickDeserialization(adapter, packet);
    if(state.first == bitsery::ReaderError::NoError && state.second)
        return true;
    return false;
}

static inline bool separate(const PacketBuffer &buffer, uint16_t &header, PayloadBuffer &payload)
{
    if(buffer.size() >= sizeof(uint16_t)) {
        payload.clear();

        // Separate header
        std::copy(buffer.cbegin(), buffer.cbegin() + sizeof(uint16_t), reinterpret_cast<uint8_t *>(&header));
        header = ENET_NET_TO_HOST_16(header);

        if(buffer.size() > sizeof(uint16_t)) {
            // Separate payload
            std::copy(buffer.cbegin() + sizeof(uint16_t), buffer.cend(), std::back_inserter(payload));
        }        

        return true;
    }

    return false;
}

template<typename packet_type>
static inline PacketBuffer serialize(const packet_type &packet)
{
    PacketBuffer buffer;

    // Serialize header
    const uint16_t header = ENET_HOST_TO_NET_16(packet_type::HEADER);
    const uint8_t *header_begin = reinterpret_cast<const uint8_t *>(&header);
    const uint8_t *header_end = reinterpret_cast<const uint8_t *>(&header + 1);
    std::copy(header_begin, header_end, std::back_inserter(buffer));

    // Serialize payload
    PayloadBuffer payload;
    auto adapter = bitsery::OutputBufferAdapter<PayloadBuffer>(payload);
    const size_t payload_size = bitsery::quickSerialization(adapter, packet);
    std::copy(payload.cbegin(), payload.cbegin() + payload_size, std::back_inserter(buffer));

    return buffer;
}
} // namespace protocol
