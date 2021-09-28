/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <game/client/comp/chunk_mesh.hpp>
#include <game/client/chunks.hpp>
#include <game/client/globals.hpp>
#include <game/shared/comp/chunk.hpp>

void ClientChunkManager::implOnClear()
{
    const auto view = cl_globals::registry.view<ChunkComponent>();
    for(const auto [entity, cp] : view.each())
        cl_globals::registry.destroy(entity);
}

void ClientChunkManager::implOnRemove(const chunkpos_t &cp, const ClientChunk &data)
{
    cl_globals::registry.destroy(data.entity);
}

ClientChunk ClientChunkManager::implOnCreate(const chunkpos_t &cp)
{
    ClientChunk data;
    data.entity = cl_globals::registry.create();
    cl_globals::registry.emplace<ChunkComponent>(data.entity, ChunkComponent(cp));
    cl_globals::registry.emplace<ChunkFlaggedForMeshingComponent>(data.entity);
    data.data.fill(NULL_VOXEL);
    return std::move(data);
}

voxel_t ClientChunkManager::implGetVoxel(const ClientChunk &data, const localpos_t &lp) const
{
    return data.data[toVoxelIdx(lp)];
}

void ClientChunkManager::implSetVoxel(ClientChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags)
{
    data->data[toVoxelIdx(lp)] = voxel;
    cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(data->entity);
    if(flags & VOXEL_SET_UPDATE_NEIGHBOURS) {
        if(ClientChunk *nc = find(cp + chunkpos_t(0, 0, 1)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
        if(ClientChunk *nc = find(cp - chunkpos_t(0, 0, 1)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
        if(ClientChunk *nc = find(cp + chunkpos_t(0, 1, 0)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
        if(ClientChunk *nc = find(cp - chunkpos_t(0, 1, 0)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
        if(ClientChunk *nc = find(cp + chunkpos_t(1, 0, 0)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
        if(ClientChunk *nc = find(cp - chunkpos_t(1, 0, 0)))
            cl_globals::registry.emplace_or_replace<ChunkFlaggedForMeshingComponent>(nc->entity);
    }
}
