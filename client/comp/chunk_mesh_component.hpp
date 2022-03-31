/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <client/gl/drawcmd.hpp>
#include <client/gl/vertex_array.hpp>
#include <common/voxels.hpp>
#include <unordered_map>

struct ChunkMesh final {
    gl::Buffer ibo, vbo;
    gl::VertexArray vao;
    gl::DrawCommand cmd;
};

struct ChunkMeshComponent final {
    // Each chunk has N meshes for
    // each respective voxel type.
    std::unordered_map<VoxelType, ChunkMesh> meshes;

    ChunkMesh *find(VoxelType type)
    {
        auto it = meshes.find(type);
        if(it != meshes.cend())
            return &it->second;
        return nullptr;
    }
};

// Flag component - entities with this component
// will be closely inspected and re-meshed next frame.
struct ChunkQueuedMeshingComponent final {};
