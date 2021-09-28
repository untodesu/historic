/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/globals.hpp>
#include <game/client/network.hpp>
#include <spdlog/spdlog.h>

void network::init()
{
    globals::peer = nullptr;
    globals::host = enet_host_create(nullptr, 1, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a client host object.");
        std::terminate();
    }
}

void network::shutdown()
{
    network::disconnect("network::shutdown");
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

bool network::connect(const std::string &host, uint16_t port)
{
    network::disconnect("network::connect");

    ENetAddress address;
    address.port = port;
    if(enet_address_set_host(&address, host.c_str()) < 0) {
        spdlog::error("Unable to find {}:{}", host, port);
        return false;
    }

    globals::peer = enet_host_connect(globals::host, &address, 2, 0);
    if(!globals::peer) {
        spdlog::error("Unable to connect to {}:{}", host, port);
        return false;
    }

    ENetEvent event;
    while(enet_host_service(globals::host, &event, 5000) > 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            spdlog::warn("network::connect: unwanted package of {} bytes", event.packet->dataLength);
            enet_packet_destroy(event.packet);
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::info("Connected to {}:{}", host, port);
            enet_host_flush(globals::host);
            return true;
        }
    }

    enet_peer_reset(globals::peer);
    globals::peer = nullptr;
    spdlog::error("Unable to establish connection with {}:{}", host, port);
    return false;
}

void network::disconnect(const std::string &reason)
{
    // UNDONE: send a protocol::Disconnect
    // package to the server so we can have
    // a nice Source-ish message in the chat.
    if(globals::peer) {
        enet_peer_disconnect(globals::peer, 0);

        ENetEvent event;
        while(enet_host_service(globals::host, &event, 2000) > 0) {
            enet_host_flush(globals::host);
            
            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                spdlog::warn("network::connect: unwanted package of {} bytes", event.packet->dataLength);
                enet_packet_destroy(event.packet);
                continue;
            }

            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected ({}).", reason);
                globals::peer = nullptr;
                return;
            }
        }

        enet_peer_reset(globals::peer);
        globals::peer = nullptr;
    }
}

bool network::event(ENetEvent &event, size_t timeout)
{
    return (enet_host_service(globals::host, &event, static_cast<uint32_t>(timeout)) > 0);
}

void network::send(size_t size, const void *data, uint8_t channel, uint32_t flags)
{
    enet_peer_send(globals::peer, channel, enet_packet_create(data, size, flags));
    enet_host_flush(globals::host);
}
