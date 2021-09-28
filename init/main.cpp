/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/filesystem.hpp>
#include <enet/enet.h>
#include <game/client/client_app.hpp>
#include <game/server/server_app.hpp>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
    try {
        spdlog::logger *lp = spdlog::default_logger_raw();
        lp->sinks().clear();
        lp->sinks().push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
        lp->set_pattern("[%H:%M:%S] %^[%L]%$ %v");
        lp->set_level(spdlog::level::trace);
    }
    catch(const spdlog::spdlog_ex &ex) {
        // Fallback to iostream
        std::cerr << ex.what() << std::endl;
        std::terminate();
    }

    fs::init();
    fs::setRoot("dist"); // keep the repo clean

    if(enet_initialize() < 0) {
        spdlog::error("Unable to initialize ENet.");
        std::terminate();
    }    

#if defined(VGAME_CLIENT)
    client_app::run();
#elif defined(VGAME_SERVER)
    server_app::run();
#else
    #error No side defined
#endif

    enet_deinitialize();

    fs::shutdown();

    return 0;
}

