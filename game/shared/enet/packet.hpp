/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <algorithm>
#include <common/math/types.hpp>
#include <enet/enet.h>
#include <utility>

namespace enet
{
class PacketRef {
public:
    PacketRef();
    PacketRef(const PacketRef &rhs) = delete;
    PacketRef(PacketRef &&rhs);
    PacketRef(ENetPacket *packet);
    PacketRef &operator=(const PacketRef &rhs) = delete;
    PacketRef &operator=(PacketRef &&rhs);
    size_t size() const;
    bool write(size_t offset, size_t size, const void *data);
    bool read(size_t offset, size_t size, void *data) const;
    ENetPacket *get() const;
    ENetPacket *invalidate();

protected:
    ENetPacket *packet;
};

class Packet final : public PacketRef {
public:
    Packet(ENetPacket *packet);
    Packet(size_t size, uint32_t flags, const void *data);
    ~Packet();
};
} // namespace enet

inline enet::PacketRef::PacketRef()
    : packet(nullptr)
{

}

inline enet::PacketRef::PacketRef(enet::PacketRef &&rhs)
    : packet(rhs.packet)
{
    rhs.packet = nullptr;
}

inline enet::PacketRef::PacketRef(ENetPacket *packet)
    : packet(packet)
{

}

inline enet::PacketRef &enet::PacketRef::operator=(enet::PacketRef &&rhs)
{
    enet::PacketRef copy(std::move(rhs));
    std::swap(packet, copy.packet);
    return *this;
}

inline size_t enet::PacketRef::size() const
{
    return packet ? packet->dataLength : 0;
}

inline bool enet::PacketRef::write(size_t offset, size_t size, const void *data)
{
    if(!packet || offset + size > packet->dataLength)
        return false;
    const uint8_t *ptr = reinterpret_cast<const uint8_t *>(data);
    std::copy(ptr, ptr + size, packet->data + offset);
    return true;
}

inline bool enet::PacketRef::read(size_t offset, size_t size, void *data) const
{
    if(!packet || offset + size > packet->dataLength)
        return false;
    uint8_t *ptr = reinterpret_cast<uint8_t *>(data);
    std::copy(packet->data + offset, packet->data + offset + size, ptr);
    return true;
}

inline ENetPacket *enet::PacketRef::get() const
{
    return packet;
}

inline ENetPacket *enet::PacketRef::invalidate()
{
    // Packets sent via Host::broadcast()
    // or Peer::send() are discarded automatically.
    ENetPacket *tmp = packet;
    packet = nullptr;
    return tmp;
}

inline enet::Packet::Packet(ENetPacket *packet)
{
    this->packet = packet;
}

inline enet::Packet::Packet(size_t size, uint32_t flags, const void *data)
{
    packet = enet_packet_create(data, size, flags);
}

inline enet::Packet::~Packet()
{
    if(!packet)
        return;
    enet_packet_destroy(packet);
}
