/*
 * chunks.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <server/chunks.hpp>
#include <server/globals.hpp>
#include <shared/components/chunk.hpp>

void ServerChunkManager::implOnClear()
{
    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, cp] : view.each())
        globals::registry.destroy(entity);
}

bool ServerChunkManager::implOnRemove(const chunkpos_t &cp, ServerChunk &data)
{
    if(--data.refcount > 1)
        return false;
    globals::registry.destroy(data.entity);
    return true;
}

ServerChunk ServerChunkManager::implOnCreate(const chunkpos_t &cp, voxel_set_flags_t)
{
    ServerChunk data;
    data.entity = globals::registry.create();
    globals::registry.emplace<ChunkComponent>(data.entity, ChunkComponent(cp));
    data.data.fill(NULL_VOXEL);
    data.refcount = 1;
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

