/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/config.hpp>

void ClientConfig::impl_postRead()
{
    window.width = toml["window"]["width"].value_or(640);
    window.height = toml["window"]["height"].value_or(480);
    window.fullscreen = toml["window"]["fullscreen"].value_or(false);
    window.vsync = toml["window"]["vsync"].value_or(true);
}

void ClientConfig::impl_preWrite()
{
    toml = toml::table {{
        { "window", toml::table {{
            { "width", window.width },
            { "height", window.height },
            { "fullscreen", window.fullscreen },
            { "vsync", window.vsync }
        }}}
    }};
}
