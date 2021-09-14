/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/client_world.hpp>
#include <spdlog/spdlog.h>

static entt::registry clw_registry;

void client_world::init()
{
    spdlog::debug("Initializing world");
    clw_registry.clear();

    // UNDONE: local player entity.
}

void client_world::shutdown()
{
    clw_registry.clear();
}

entt::registry &client_world::registry()
{
    return clw_registry;
}
