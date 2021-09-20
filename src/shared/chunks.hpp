/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/world.hpp>
#include <unordered_map>

// While ECS is supposed to only manage these objects,
// Voxel data should be stored somewhere else because
// Voxelius uses thread pool to generate meshes. Copying
// up to five whole chunks per task is a terrible idea.
class ChunkManager final {
public:
    ChunkManager();
    void clear();
    bool set(const voxelpos_t &vp, voxel_t voxel);
    void forceSet(const voxelpos_t &vp, voxel_t voxel);
    voxel_array_t *find(const chunkpos_t &cp);
    voxel_array_t *findOrCreate(const chunkpos_t &cp);

private:
    std::unordered_map<chunkpos_t, voxel_array_t> data;
};
