/*
 * network.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <common/math/types.hpp>
#include <string>

namespace cl_network
{
void init();
void shutdown();
bool connect(const std::string &host, uint16_t port);
void disconnect(const std::string &reason);
void update();
entt::entity createEntity(uint32_t network_id);
entt::entity findEntity(uint32_t network_id);
void removeEntity(uint32_t network_id);
} // namespace cl_network

namespace network = cl_network;
