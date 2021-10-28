/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <server/chunks.hpp>
#include <server/globals.hpp>
#include <shared/comp/chunk.hpp>

void ServerChunkManager::implOnClear()
{
    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, cp] : view.each())
        globals::registry.destroy(entity);
}

void ServerChunkManager::implOnRemove(const chunkpos_t &cp, const ServerChunk &data)
{
    globals::registry.destroy(data.entity);
}

ServerChunk ServerChunkManager::implOnCreate(const chunkpos_t &cp, voxel_set_flags_t)
{
    ServerChunk data;
    data.entity = globals::registry.create();
    globals::registry.emplace<ChunkComponent>(data.entity, ChunkComponent(cp));
    data.data.fill(NULL_VOXEL);
    return std::move(data);
}

voxel_t ServerChunkManager::implGetVoxel(const ServerChunk &data, const localpos_t &lp) const
{
    return data.data[toVoxelIdx(lp)];
}

void ServerChunkManager::implSetVoxel(ServerChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags)
{
    // TODO: if the chunk is loaded, broadcast a packet
    data->data[toVoxelIdx(lp)] = voxel;
}

