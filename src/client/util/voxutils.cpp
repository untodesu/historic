/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/globals.hpp>
#include <client/util/voxutils.hpp>
#include <shared/comp/chunk.hpp>

bool voxutils::chunk(const chunkpos_t &cp, voxel_array_t &out)
{
    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, chunk] : view.each()) {
        if(chunk.position != cp)
            continue;
        std::copy(chunk.data.cbegin(), chunk.data.cend(), out.begin());
        return true;
    }

    return false;
}

voxel_array_t *voxutils::chunk(const chunkpos_t &cp)
{
    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, chunk] : view.each()) {
        if(chunk.position != cp)
            continue;
        return &chunk.data;
    }

    return nullptr;
}
