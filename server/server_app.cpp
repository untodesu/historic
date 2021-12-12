/*
 * server_app.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <csignal>
#include <server/config.hpp>
#include <server/game.hpp>
#include <server/globals.hpp>
#include <server/server_app.hpp>
#include <server/network.hpp>
#include <shared/protocol/protocol.hpp>
#include <common/util/clock.hpp>
#include <spdlog/spdlog.h>
#include <thread>

constexpr const float TICK_DT = 1.0f / protocol::DEFAULT_TICKRATE;

static void onSIGINT(int)
{
    spdlog::warn("SIGINT received");
    globals::running = false;
}

void server_app::run()
{
    globals::config.read("server.toml");

    globals::running = true;

    std::signal(SIGINT, &onSIGINT);

    network::init();
    game::init();
    
    game::postInit();

    globals::curtime = 0.0f;
    globals::ticktime = 0.0f;
    globals::num_ticks = 0;

    const std::chrono::microseconds tick_us(static_cast<size_t>(TICK_DT * 1.0e6f));

    ChronoClock<std::chrono::system_clock> clock;
    std::chrono::system_clock::time_point time_accum = clock.now();

    spdlog::info("net.maxplayers = {}", globals::config.net.maxplayers);
    spdlog::info("net.port = {}", globals::config.net.port);

    while(globals::running) {
        std::chrono::system_clock::time_point time_now = clock.now();
        globals::curtime = util::seconds<float>(time_now.time_since_epoch());
        globals::ticktime = util::seconds<float>(clock.restart());

        if(globals::ticktime < TICK_DT) {
            if(size_t drop = static_cast<size_t>(util::seconds<float>(time_now - time_accum) / TICK_DT)) {
                spdlog::warn("Dropping {} ticks", drop);
                time_accum = time_now;
            }
        }

        network::update();

        game::update();
        globals::num_ticks++;

        std::this_thread::sleep_until(time_accum += tick_us);
    }

    game::shutdown();
    spdlog::info("Server shutdown after {} ticks", globals::num_ticks);

    network::shutdown();

    globals::config.write("server.toml");
}
