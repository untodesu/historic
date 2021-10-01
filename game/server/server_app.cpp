/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <enet/enet.h>
#include <exception>
#include <game/server/game.hpp>
#include <game/server/globals.hpp>
#include <game/server/server_app.hpp>
#include <game/shared/protocol/protocol.hpp>
#include <game/shared/util/clock.hpp>
#include <spdlog/spdlog.h>
#include <thread>

void server_app::run()
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = protocol::DEFAULT_PORT;

    globals::host = enet_host_create(&address, 16, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a server host object.");
        std::terminate();
    }

    game::init();

    game::postInit();

    globals::curtime = 0.0f;
    globals::ticktime = 0.0f;
    globals::num_ticks = 0;

    ChronoClock<std::chrono::system_clock> ticktime_clock;
    for(;;) {
        globals::curtime = util::seconds<float>(ticktime_clock.now().time_since_epoch());
        globals::ticktime = util::seconds<float>(ticktime_clock.restart());
        game::update();
        globals::num_ticks++;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    spdlog::info("Server shutdown after {} ticks", globals::num_ticks);
    game::shutdown();

    enet_host_destroy(globals::host);
    globals::host = nullptr;
}
