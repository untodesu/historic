/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <shared/chunks.hpp>

ChunkManager::ChunkManager()
    : data()
{
    
}

void ChunkManager::clear()
{
    data.clear();
}

voxel_array_t *ChunkManager::find(const chunkpos_t &cp)
{
    const auto &it = data.find(cp);
    if(it != data.cend())
        return &it->second;
    return nullptr;
}

voxel_array_t *ChunkManager::findOrCreate(const chunkpos_t &cp)
{
    const auto &it = data.find(cp);
    if(it != data.cend())
        return &it->second;
    return &(data[cp] = voxel_array_t());
}
