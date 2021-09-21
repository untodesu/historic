/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/comp/voxel_mesh.hpp>
#include <client/client_chunks.hpp>
#include <client/globals.hpp>

void ClientChunkManager::implOnClear()
{
    const auto view = globals::registry.view<chunkpos_t>();
    for(const auto [entity, cp] : view.each())
        globals::registry.destroy(entity);
}

void ClientChunkManager::implOnRemove(const chunkpos_t &cp, const ClientChunk &data)
{
    globals::registry.destroy(data.entity);
}

ClientChunk ClientChunkManager::implOnCreate(const chunkpos_t &cp)
{
    ClientChunk data;
    data.entity = globals::registry.create();
    globals::registry.emplace<chunkpos_t>(data.entity, cp);
    globals::registry.emplace<NeedsVoxelMeshComponent>(data.entity);
    data.data.fill(NULL_VOXEL);
    return std::move(data);
}

voxel_t ClientChunkManager::implGetVoxel(const ClientChunk &data, const localpos_t &lp) const
{
    return data.data[toVoxelIdx(lp)];
}

void ClientChunkManager::implSetVoxel(ClientChunk *data, const localpos_t &lp, voxel_t voxel)
{
    data->data[toVoxelIdx(lp)] = voxel;
}
