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
#include <game/shared/protocol/packets/client/handshake.hpp>
#include <game/shared/protocol/packets/client/login_start.hpp>
#include <game/shared/protocol/packets/client/request_chunks.hpp>
#include <game/shared/protocol/packets/client/request_spawn.hpp>
#include <game/shared/protocol/packets/client/request_voxels.hpp>
#include <game/shared/protocol/packets/server/chunk_data.hpp>
#include <game/shared/protocol/packets/server/client_spawn.hpp>
#include <game/shared/protocol/packets/server/login_success.hpp>
#include <game/shared/protocol/packets/server/voxels_checksum.hpp>
#include <game/shared/protocol/packets/server/voxels_face.hpp>
#include <game/shared/protocol/packets/shared/disconnect.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

constexpr static const char *DEFAULT_DISCONNECT_MESSAGE = "Disconnected";

static protocol::ClientState client_state = protocol::ClientState::DISCONNECTED;
static uint32_t session_id = 0;

using packet_handler_t = void(*)(const std::vector<uint8_t> &);
static const std::unordered_map<uint16_t, packet_handler_t> packet_handlers = {
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);
            spdlog::info("Disconnected: {}", packet.reason);
            enet_peer_disconnect(globals::peer, 0);
            client_state = protocol::ClientState::DISCONNECTED;
        }
    },
    {
        protocol::packets::LoginSuccess::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::LoginSuccess packet;
            protocol::deserialize(payload, packet);
            
        }
    }
};

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

            client_state = protocol::ClientState::LOGGING_IN;
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

            const auto it = packet_handlers.find(type);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} received from server", type);
                continue;
            }

            it->second(payload);
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
