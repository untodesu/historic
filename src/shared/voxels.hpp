/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/world.hpp>
#include <unordered_map>

enum class VoxelType {
    GAS,            // Kind of a liquid voxel, but in reverse. Reserved.
    SOLID,          // A solid voxel. Rendered through voxel_renderer.
    SOLID_ENTITY,   // A solid voxel entity. Reserved.
    SOLID_FLORA,    // A solid voxel flora. Reserved.
    LIQUID,         // A liquid voxel. Reserved.
};

using voxel_face_t = uint16_t;
constexpr static const voxel_face_t VOXEL_FACE_LF = (1 << 0);
constexpr static const voxel_face_t VOXEL_FACE_RT = (1 << 1);
constexpr static const voxel_face_t VOXEL_FACE_FT = (1 << 2);
constexpr static const voxel_face_t VOXEL_FACE_BK = (1 << 3);
constexpr static const voxel_face_t VOXEL_FACE_UP = (1 << 4);
constexpr static const voxel_face_t VOXEL_FACE_DN = (1 << 5);
constexpr static const voxel_face_t VOXEL_FACE_SIDES = VOXEL_FACE_LF | VOXEL_FACE_RT | VOXEL_FACE_FT | VOXEL_FACE_BK;
constexpr static const voxel_face_t VOXEL_FACE_SOLID = VOXEL_FACE_LF | VOXEL_FACE_RT | VOXEL_FACE_FT | VOXEL_FACE_BK | VOXEL_FACE_UP | VOXEL_FACE_DN;

struct VoxelFaceInfo final {
    voxel_face_t mask;
    std::string texture;
};

struct VoxelInfo final {
    VoxelType type;
    voxel_face_t transparency;
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

constexpr static inline const voxel_face_t backVoxelFace(voxel_face_t face)
{
    if(face & VOXEL_FACE_LF)
        return VOXEL_FACE_RT;
    if(face & VOXEL_FACE_RT)
        return VOXEL_FACE_LF;
    if(face & VOXEL_FACE_FT)
        return VOXEL_FACE_BK;
    if(face & VOXEL_FACE_BK)
        return VOXEL_FACE_FT;
    if(face & VOXEL_FACE_UP)
        return VOXEL_FACE_DN;
    if(face & VOXEL_FACE_DN)
        return VOXEL_FACE_UP;
    return 0;
}

static inline void unwrapVoxelFaces(voxel_face_t face, std::vector<voxel_face_t> &unwrap)
{
    unwrap.clear();
    if(voxel_face_t f = face & VOXEL_FACE_LF)
        unwrap.push_back(f);
    if(voxel_face_t f = face & VOXEL_FACE_RT)
        unwrap.push_back(f);
    if(voxel_face_t f = face & VOXEL_FACE_FT)
        unwrap.push_back(f);
    if(voxel_face_t f = face & VOXEL_FACE_BK)
        unwrap.push_back(f);
    if(voxel_face_t f = face & VOXEL_FACE_UP)
        unwrap.push_back(f);
    if(voxel_face_t f = face & VOXEL_FACE_DN)
        unwrap.push_back(f);
}
