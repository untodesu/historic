/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <exception>
#include <game/client/game.hpp>
#include <game/client/gbuffer.hpp>
#include <game/client/globals.hpp>
#include <game/shared/protocol/packets/disconnect.hpp>
#include <game/shared/protocol/packets/handshake.hpp>
#include <game/shared/protocol/packets/login_start.hpp>
#include <game/shared/protocol/packets/login_success.hpp>
#include <spdlog/spdlog.h>

enum class ClientStatus {
    DISCONNECTED,   // The client is disconnected
    LOGGING_IN,     // The client is awaiting for a LoginSuccess packet
    CONNECTED,      // The client is connected and ready for playing
};

constexpr static const char *DEFAULT_DISCONNECT_MESSAGE = "Disconnected";

static ClientStatus status = ClientStatus::DISCONNECTED;

void cl_game::init()
{
    // We literally do nothing here at the moment
}

void cl_game::postInit()
{
    cl_game::connect("localhost", protocol::DEFAULT_PORT);
}

void cl_game::shutdown()
{
    cl_game::disconnect("cl_game::shutdown");
}

void cl_game::modeChange(int width, int height)
{
    // We literally do nothing here at the moment
}

bool cl_game::connect(const std::string &host, uint16_t port)
{
    cl_game::disconnect(DEFAULT_DISCONNECT_MESSAGE);

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
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::debug("Logging in...");

            // Send a HandShake packet
            {
                protocol::packets::Handshake packet;
                const std::vector<uint8_t> pbuf = protocol::serialize(packet);
                enet_peer_send(globals::peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
            }

            // Send a LoginStart packet
            // UNDONE: change the username
            {
                protocol::packets::LoginStart packet;
                packet.username = "Kirill";
                const std::vector<uint8_t> pbuf = protocol::serialize(packet);
                enet_peer_send(globals::peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
            }

            status = ClientStatus::LOGGING_IN;
            return true;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            spdlog::debug("Unwanted (right now) packet with the size of {:.02f} KiB", static_cast<float>(event.packet->dataLength) / 1024.0f);
            enet_packet_destroy(event.packet);
            continue;
        }
    }

    enet_peer_reset(globals::peer);
    globals::peer = nullptr;

    spdlog::error("Unable to establish connection with {}:{}", host, port);
    return false;
}

void cl_game::disconnect(const std::string &reason)
{
    if(globals::peer) {
        // Send a Disconnect event
        {
            protocol::packets::Disconnect packet;
            packet.reason = reason;
            const std::vector<uint8_t> pbuf = protocol::serialize(packet);
            enet_peer_send(globals::peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
        }

        enet_peer_disconnect_later(globals::peer, 0);

        ENetEvent event;
        while(enet_host_service(globals::host, &event, 5000) > 0) {
            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected.");
                break;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                spdlog::debug("Unwanted (right now) packet with the size of {:.02f} KiB", static_cast<float>(event.packet->dataLength) / 1024.0f);
                enet_packet_destroy(event.packet);
                continue;
            }
        }

        enet_peer_reset(globals::peer);
        globals::peer = nullptr;
    }
}

void cl_game::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            const std::vector<uint8_t> packet = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t type;
            std::vector<uint8_t> payload;
            if(!protocol::split(packet, type, payload)) {
                spdlog::warn("Invalid packet format!");
                continue;
            }

            // Handle Disconnect
            if(type == protocol::packets::Disconnect::id) {
                protocol::packets::Disconnect packet;
                protocol::deserialize(payload, packet);
                spdlog::info("Disconnected: {}", packet.reason);
                enet_peer_reset(globals::peer);
                status = ClientStatus::DISCONNECTED;
                break;
            }

            // Handle LoginSuccess
            if(type == protocol::packets::LoginSuccess::id) {
                protocol::packets::LoginSuccess packet;
                protocol::deserialize(payload, packet);
                spdlog::info("Logged in with ID={}", packet.session_id);
                status = ClientStatus::CONNECTED;
                continue;
            }

            // TODO: Handle KeepAlive
            // TODO: Handle VoxelDefEntry (ClientStatus::RETREIVING_SERVER_INFO)
            // TODO: Handle PlayerInfo (ClientStatus::RETREIVING_SERVER_INFO)
            // TODO: Handle ChunkVoxelData (ClientStatus::LOADING_WORLD & ClientStatus::CONNECTED)
            // etc. etc.
        }
    }

    // We literally do nothing here at the moment
}

void cl_game::draw()
{
    // We literally do nothing here at the moment
}

void cl_game::imgui()
{
    // We literally do nothing here at the moment
}
