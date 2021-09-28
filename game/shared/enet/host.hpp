/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/enet/address.hpp>
#include <game/shared/enet/peer.hpp>

namespace enet
{
class Host final {
public:
    Host();
    Host(const Host &rhs) = delete;
    Host(Host &&rhs);
    Host(const ENetAddress *address, size_t peers, size_t channels, size_t ibw, size_t obw);
    Host(const Address &address, size_t peers, size_t channels, size_t ibw, size_t obw);
    Host &operator=(const Host &rhs) = delete;
    Host &operator=(Host &&rhs);
    ~Host();
    void create(const ENetAddress *address, size_t peers, size_t channels, size_t ibw, size_t obw);
    void destroy();
    int service(ENetEvent &event, size_t timeout);
    void broadcast(uint8_t channel, PacketRef &packet);
    void flush();
    ENetPeer *connect(const Address &address, size_t channels);
    ENetHost *get() const;

private:
    ENetHost *host;
};
} // namespace enet

inline enet::Host::Host()
    : host(nullptr)
{

}

inline enet::Host::Host(enet::Host &&rhs)
    : host(rhs.host)
{
    rhs.host = nullptr;
}

inline enet::Host::Host(const ENetAddress *address, size_t peers, size_t channels, size_t ibw, size_t obw)
    : host(nullptr)
{
    create(address, peers, channels, ibw, obw);
}

inline enet::Host::Host(const enet::Address &address, size_t peers, size_t channels, size_t ibw, size_t obw)
    : host(nullptr)
{
    create(address.get(), peers, channels, ibw, obw);
}

inline enet::Host &enet::Host::operator=(enet::Host &&rhs)
{
    enet::Host copy(std::move(rhs));
    std::swap(host, copy.host);
    return *this;
}

inline enet::Host::~Host()
{
    destroy();
}

inline void enet::Host::create(const ENetAddress *address, size_t peers, size_t channels, size_t ibw, size_t obw)
{
    destroy();
    host = enet_host_create(address, peers, channels, static_cast<uint32_t>(ibw), static_cast<uint32_t>(obw));
}

inline void enet::Host::destroy()
{
    if(host) {
        enet_host_destroy(host);
        host = nullptr;
    }
}

inline int enet::Host::service(ENetEvent &event, size_t timeout)
{
    if(!host)
        return -1;
    return enet_host_service(host, &event, static_cast<uint32_t>(timeout));
}

inline void enet::Host::broadcast(uint8_t channel, enet::PacketRef &packet)
{
    if(!host)
        return;
    enet_host_broadcast(host, channel, packet.invalidate());
}

inline void enet::Host::flush()
{
    if(!host)
        return;
    enet_host_flush(host);
}

inline ENetPeer *enet::Host::connect(const enet::Address &address, size_t channels)
{
    if(!host)
        return nullptr;
    return enet_host_connect(host, address.get(), channels, 0);
}

inline ENetHost *enet::Host::get() const
{
    return host;
}
