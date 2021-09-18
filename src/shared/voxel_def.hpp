/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <math/types.hpp>
#include <shared/world.hpp>
#include <string>
#include <vector>

using voxel_face_t = uint16_t;
constexpr static const voxel_face_t VOXEL_FACE_LF = (1 << 0);
constexpr static const voxel_face_t VOXEL_FACE_RT = (1 << 1);
constexpr static const voxel_face_t VOXEL_FACE_FT = (1 << 2);
constexpr static const voxel_face_t VOXEL_FACE_BK = (1 << 3);
constexpr static const voxel_face_t VOXEL_FACE_UP = (1 << 4);
constexpr static const voxel_face_t VOXEL_FACE_DN = (1 << 5);
constexpr static const voxel_face_t VOXEL_FACE_SIDES = VOXEL_FACE_LF | VOXEL_FACE_RT | VOXEL_FACE_FT | VOXEL_FACE_BK;
constexpr static const voxel_face_t VOXEL_FACE_SOLID = VOXEL_FACE_LF | VOXEL_FACE_RT | VOXEL_FACE_FT | VOXEL_FACE_BK | VOXEL_FACE_UP | VOXEL_FACE_DN;

enum class VoxelType {
    SOLID,          // A solid voxel. Rendered through voxel_renderer.
    SOLID_ENTITY,   // A solid voxel entity. Reserved.
    SOLID_FLORA,    // A solid voxel flora. Reserved.
    LIQUID,         // A liquid voxel. Reserved.
};

struct VoxelFaceInfo final {
    voxel_face_t mask;
    std::string texture;
};

struct VoxelInfo final {
    voxel_face_t visibility;
    voxel_face_t transparency;
    VoxelType type;
    std::vector<VoxelFaceInfo> faces;
};

namespace voxel_def
{
void add(voxel_t voxel, const VoxelInfo &info);
bool exists(voxel_t voxel);
const VoxelInfo *get(voxel_t voxel);
size_t listSize();
const voxel_t *list();
} // namespace voxel_def
