/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/enet/init.hpp>
#include <enet/enet.h>
#include <exception>

void enet::init()
{
    if(enet_initialize() >= 0)
        return;
    std::terminate();
}

void enet::shutdown()
{
    enet_deinitialize();
}
