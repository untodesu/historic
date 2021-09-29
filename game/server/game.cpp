/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/game.hpp>
#include <game/server/globals.hpp>
#include <game/shared/protocol/packets/disconnect.hpp>
#include <game/shared/protocol/packets/handshake.hpp>
#include <game/shared/protocol/packets/login_start.hpp>
#include <game/shared/protocol/packets/login_success.hpp>
#include <spdlog/spdlog.h>


struct PeerData final {
    uint32_t id;
    protocol::ServerState state;
    std::string username;
};

void sv_game::init()
{

}

void sv_game::postInit()
{

}

void sv_game::shutdown()
{

}

void sv_game::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::info("Someone connected...");
            PeerData *data = new PeerData;
            data->id = 0;
            data->state = protocol::ServerState::AWAIT_HANDSHAKE;
            event.peer->data = data;
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
            spdlog::info("Someone disconnected...");
            delete reinterpret_cast<PeerData *>(event.peer->data);
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            PeerData *data = reinterpret_cast<PeerData *>(event.peer->data);

            const std::vector<uint8_t> pbuf = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t type;
            std::vector<uint8_t> payload;
            if(!protocol::split(pbuf, type, payload)) {
                spdlog::warn("Invalid packet format!");
                continue;
            }

            // Handle Handshake
            if(type == protocol::packets::Handshake::id) {
                protocol::packets::Handshake packet;
                protocol::deserialize(payload, packet);
                spdlog::info("Handshake version {}", packet.version);
                data->state = protocol::ServerState::AWAIT_LOGIN;
                continue;
            }

            // Handle LoginStart
            if(type == protocol::packets::LoginStart::id) {
                protocol::packets::LoginStart packet;
                protocol::deserialize(payload, packet);
                spdlog::info("LoginStart({})", packet.username);

                // Send LoginSuccess
                {
                    protocol::packets::LoginSuccess packet;
                    packet.session_id = data->id;
                    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
                    enet_peer_send(event.peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
                }

                data->state = protocol::ServerState::PLAYING;
                data->username = packet.username;
                continue;
            }

            // Handle disconnect
            if(type == protocol::packets::Disconnect::id) {
                protocol::packets::Disconnect packet;
                protocol::deserialize(payload, packet);
                spdlog::info("{} ({}) disconnected ({})", data->username, data->id, packet.reason);
                enet_peer_disconnect(event.peer, 0);
                continue;
            }
        }
    }
}
