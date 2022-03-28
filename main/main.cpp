/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cctype>
#include <client/client_app.hpp>
#include <common/cmdline.hpp>
#include <common/fs.hpp>
#include <common/math/constexpr.hpp>
#include <common/util/spdlog_sinks.hpp>
#include <enet/enet.h>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main(int argc, char **argv)
{
    try {
        spdlog::logger *lp = spdlog::default_logger_raw();
        lp->sinks().clear();
        lp->sinks().push_back(util::makeSinkSimple<spdlog::sinks::stderr_color_sink_mt>());
    }
    catch(const spdlog::spdlog_ex &ex) {
        // fall back to iostream and die
        std::cerr << "spdlog: " << ex.what() << std::endl;
        std::terminate();
    }

    cmdline::initialize(argc, argv);

#if defined(NDEBUG)
    const bool is_debug = cmdline::find("debug");
#else
    const bool is_debug = true;
#endif

    spdlog::set_level(spdlog::level::info);
    if(cmdline::find("trace")) {
        spdlog::set_level(spdlog::level::trace);
        spdlog::info("setting log_level to TRACE (cmdline)");
    }
    else if(is_debug) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::info("setting log_level to DEBUG");
    }

    if(!fs::setSearchRoot("res")) {
        spdlog::error("fs: unable to find or create search root directory");
        std::terminate();
    }

    if(enet_initialize() != 0) {
        spdlog::critical("enet: failed to initialize");
        std::terminate();
    }

#if defined(VGAME_CLIENT)
    client_app::run();
#else
#error Amogus
#endif

    enet_deinitialize();

    fs::shutdown();

    cmdline::shutdown();

    return 0;
}


