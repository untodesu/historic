/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <shared/comp/network.hpp>
#include <client/globals.hpp>
#include <client/net_entities.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

static std::unordered_map<uint32_t, entt::entity> data;

void net_entities::clear()
{
    const auto view = globals::registry.view<NetworkComponent>();
    for(const auto [entity, net] : view.each())
        globals::registry.destroy(entity);
    data.clear();
}

entt::entity net_entities::create(uint32_t network_id)
{
    const auto it = data.find(network_id);
    if(it != data.cend()) {
        spdlog::warn("Network entity {} already exists!", network_id);
        return it->second;
    }

    entt::entity entity = globals::registry.create();
    NetworkComponent &net = globals::registry.emplace<NetworkComponent>(entity);
    net.network_id = network_id;
    return data[network_id] = entity;
}

entt::entity net_entities::find(uint32_t network_id)
{
    const auto it = data.find(network_id);
    if(it != data.cend())
        return it->second;
    return entt::null;
}

void net_entities::remove(uint32_t network_id)
{
    const auto it = data.find(network_id);
    if(it != data.cend()) {
        globals::registry.destroy(it->second);
        data.erase(it);
    }
}
