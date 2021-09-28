/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/globals.hpp>
#include <game/server/network.hpp>
#include <game/shared/protocol/shared/disconnect.hpp>
#include <game/shared/protocol/util.hpp>
#include <spdlog/spdlog.h>

void network::init(size_t peers, uint16_t port)
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = port;

    globals::host = enet_host_create(&address, peers, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a server host object.");
        std::terminate();
    }
}

void network::shutdown()
{
    // NOTE: should we kick all the peers before?
    // NOTE: should we store the peers somewhere?
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

void network::disconnect(ENetPeer *peer, const std::string &reason)
{
    const std::vector<uint8_t> dpbuf = protocol::serialize(protocol::Disconnect(reason));
    enet_peer_send(peer, 0, enet_packet_create(dpbuf.data(), dpbuf.size(), ENET_PACKET_FLAG_RELIABLE));
    enet_host_flush(globals::host);

    // TODO: broadcast a message to everyone else.

    // Oh you didn't receive my disconnect packet?
    // Too bad! I am still kicking you out!
    // Seriously though there should be an another way.
    enet_peer_reset(peer);
}

bool network::event(ENetEvent &event, size_t timeout)
{
    return (enet_host_service(globals::host, &event, static_cast<uint32_t>(timeout)) > 0);
}

void network::broadcast(size_t size, const void *data, uint8_t channel, uint32_t flags)
{
    enet_host_broadcast(globals::host, channel, enet_packet_create(data, size, flags));
    enet_host_flush(globals::host);
}
