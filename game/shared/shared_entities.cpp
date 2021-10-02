/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/shared/comp/entity_id.hpp>
#include <game/shared/shared_entities.hpp>

SharedEntityManager::SharedEntityManager(entt::registry &registry)
    : registry(&registry), entities()
{

}

void SharedEntityManager::clear()
{
    for(const auto it : entities)
        registry->destroy(it.second);
    entities.clear();
}

entt::entity SharedEntityManager::create(uint64_t id)
{
    const auto it = entities.find(id);
    if(it != entities.cend())
        return it->second;
    entt::entity entity = registry->create();
    registry->emplace<EntityIDComponent>(entity).id = id;
    return (entities[id] = entity);
}

entt::entity SharedEntityManager::get(uint64_t id)
{
    const auto it = entities.find(id);
    if(it != entities.cend())
        return it->second;
    return entt::null;
}

void SharedEntityManager::destroy(uint64_t id)
{
    const auto it = entities.find(id);
    if(it != entities.cend()) {
        registry->destroy(it->second);
        entities.erase(it);
    }
}
