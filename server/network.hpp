/*
 * network.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once

namespace sv_network
{
void init();
void shutdown();
void update();
} // namespace sv_network

namespace network = sv_network;
