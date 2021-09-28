/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/math/types.hpp>
#include <enet/enet.h>
#include <string>

namespace enet
{
class Address final {
public:
    Address() = default;
    Address(const ENetAddress &address);
    void setHost(uint32_t host);
    void setHost(const std::string &host);
    void setPort(uint16_t port);
    const ENetAddress *get() const;

private:
    ENetAddress address;
};

constexpr static const ENetAddress *NULL_ADDRESS = nullptr;
} // namespace enet

inline enet::Address::Address(const ENetAddress &address)
    : address(address)
{

}

inline void enet::Address::setHost(uint32_t host)
{
    address.host = host;
}

inline void enet::Address::setHost(const std::string &host)
{
    enet_address_set_host(&address, host.c_str());
}

inline void enet::Address::setPort(uint16_t port)
{
    address.port = port;
}

inline const ENetAddress *enet::Address::get() const
{
    return &address;
}
