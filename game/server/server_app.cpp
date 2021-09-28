/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/chunks.hpp>
#include <game/server/globals.hpp>
#include <game/server/network.hpp>
#include <game/server/server_app.hpp>
#include <game/shared/protocol/client/handshake.hpp>
#include <game/shared/protocol/util.hpp>
#include <spdlog/spdlog.h>
#include <thread>

void server_app::run()
{
    network::init(1, 24000);

    ENetEvent event;
    for(;;) {
        while(network::event(event)) {
            if(event.type == ENET_EVENT_TYPE_CONNECT) {
                spdlog::info("Connected!");
                continue;
            }

            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected!");
                continue;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                spdlog::info("Data!");

                const std::vector<uint8_t> packet(event.packet->data, event.packet->data + event.packet->dataLength);

                uint16_t type;
                std::vector<uint8_t> payload;
                if(protocol::split(packet, type, payload)) {
                    if(type == protocol::Handshake::ID) {
                        protocol::Handshake hs;
                        if(protocol::deserialize(payload, hs)) {
                            spdlog::info("Handshake! {}", hs.version);
                        }
                    }
                }

                enet_packet_destroy(event.packet);
                continue;
            }
        }
    }

    network::shutdown();
}
