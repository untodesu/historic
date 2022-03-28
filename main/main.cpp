/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <algorithm>
#include <cctype>
#include <client/client_app.hpp>
#include <core/cmdline.hpp>
#include <core/fs.hpp>
#include <core/math/constexpr.hpp>
#include <core/util/spdlog_sinks.hpp>
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

    // allow custom log levels
    std::string log_level_str;
    if(cmdline::find("log_level", log_level_str)) {
        spdlog::warn("cmdline: setting log level to {}", log_level_str);
        spdlog::set_level(spdlog::level::from_str(log_level_str));
    }

    fs::initialize();

    std::string search_root = "res";
    cmdline::find("root", search_root);
    if(!fs::setSearchRoot(search_root)) {
        spdlog::critical("fs: unable to setup new search root: {}", search_root);
        std::terminate();
    }

    if(enet_initialize() != 0) {
        spdlog::critical("enet: failed to initialize");
        std::terminate();
    }

    client_app::run();

    enet_deinitialize();
    fs::shutdown();
    cmdline::shutdown();

    return 0;
}


