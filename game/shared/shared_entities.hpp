/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <unordered_map>

class SharedEntityManager {
public:
    SharedEntityManager(entt::registry &registry);
    void clear();
    entt::entity create(uint64_t id);
    entt::entity get(uint64_t id);
    void destroy(uint64_t id);

private:
    entt::registry *registry;
    std::unordered_map<uint64_t, entt::entity> entities;
};
