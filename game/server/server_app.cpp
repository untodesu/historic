/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/server/server_app.hpp>
#include <spdlog/spdlog.h>
#include <game/shared/enet/host.hpp>
#include <game/shared/enet/init.hpp>
#include <game/shared/packets/common.hpp>

void server_app::run()
{
    enet::Address addr;
    addr.setHost("localhost");
    addr.setPort(24000);

    enet::Host host = enet::Host(addr, 16, 2, 0, 0);
    if(!host.get()) {
        spdlog::error("SV: enet::Host::Host() failed");
        std::terminate();
    }

    ENetEvent event;
    while(host.service(event, 10000) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::info("SV: someone connected");
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
            spdlog::info("SV: someone disconnected");
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            packets::TestPacket tp;
            packets::buffer_type buffer;
            std::copy(event.packet->data, event.packet->data + event.packet->dataLength, std::back_inserter(buffer));

            const auto state = bitsery::quickDeserialization(packets::input_adapter { buffer.begin(), event.packet->dataLength }, tp);
            if(state.first == bitsery::ReaderError::NoError && state.second)
                spdlog::info("SV: received a valid TestPacket: {}, {:X}", tp.a, tp.b);

            enet_packet_destroy(event.packet);
            continue;
        }
    }
}
