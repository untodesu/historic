/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <game/shared/world.hpp>
#include <unordered_map>
#include <unordered_set>

enum class VoxelType {
    GAS,            // Kind of a liquid voxel, but in reverse. Reserved.
    SOLID,          // A solid voxel. Rendered through chunk_renderer.
    SOLID_ENTITY,   // A solid voxel entity. Reserved.
    SOLID_FLORA,    // A solid voxel flora. Reserved.
    LIQUID,         // A liquid voxel. Reserved.
};

enum class VoxelFace : uint16_t {
    LF, // west
    RT, // east
    FT, // south
    BK, // north
    UP, // up
    DN, // down
};

constexpr static inline const VoxelFace backVoxelFace(const VoxelFace face)
{
    if(face == VoxelFace::LF)
        return VoxelFace::RT;
    if(face == VoxelFace::RT)
        return VoxelFace::LF;
    if(face == VoxelFace::FT)
        return VoxelFace::BK;
    if(face == VoxelFace::BK)
        return VoxelFace::FT;
    if(face == VoxelFace::UP)
        return VoxelFace::DN;
    if(face == VoxelFace::DN)
        return VoxelFace::UP;
    return VoxelFace::LF;
}

constexpr static inline const int16_t voxelFaceNormal(const VoxelFace face)
{
    if(face == VoxelFace::LF)
        return 1;
    if(face == VoxelFace::FT)
        return 1;
    if(face == VoxelFace::DN)
        return 1;
    if(face == VoxelFace::RT)
        return -1;
    if(face == VoxelFace::BK)
        return -1;
    if(face == VoxelFace::UP)
        return -1;
    return 0;
}

constexpr static inline const bool isBackVoxelFace(const VoxelFace face)
{
    if(face == VoxelFace::LF)
        return true;
    if(face == VoxelFace::FT)
        return true;
    if(face == VoxelFace::DN)
        return true;
    return false;
}

struct VoxelFaceInfo final {
    VoxelFace face;
    std::string texture;
};

struct VoxelInfo final {
    VoxelType type;
    std::unordered_set<VoxelFace> transparency;
    std::vector<VoxelFaceInfo> faces;
};

class VoxelDef final {
public:
    using map_type = std::unordered_map<voxel_t, VoxelInfo>;
    using const_iterator = map_type::const_iterator;

public:
    VoxelDef();

    bool set(voxel_t voxel, const VoxelInfo &info);
    const VoxelInfo *tryGet(voxel_t voxel) const;

    inline const_iterator cbegin() const
    {
        return def.cbegin();
    }

    inline const_iterator cend() const
    {
        return def.cend();
    }

private:
    map_type def;
};