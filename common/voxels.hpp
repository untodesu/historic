/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/mixin.hpp>
#include <common/world.hpp>
#include <unordered_map>
#include <string>
#include <vector>

enum class VoxelType : uint8_t {
    STATIC_CUBE = 0x00,
    STATIC_FLORA = 0x01,
    STATIC_LIQUID = 0x02,
    STATIC_GAS = 0x03,

    INVALID_TYPE = 0xFF
};

enum class VoxelFace : uint8_t {
    LF = 0x00,
    RT = 0x01,
    FT = 0x02,
    BK = 0x03,
    UP = 0x04,
    DN = 0x05,
};

struct VoxelDefEntry final {
    struct Face final {
        bool transparent { false };
        std::vector<std::string> textures;
    };

    VoxelType type { VoxelType::INVALID_TYPE };
    std::unordered_map<VoxelFace, Face> faces;
};

class VoxelDef;
namespace detail
{
class VoxelDefFaceBuilder;
class VoxelDefEntryBuilder final : public mixin::NoCopy, public mixin::NoMove {
    friend class VoxelDefFaceBuilder;

public:
    VoxelDefEntryBuilder(VoxelDef *owner, voxel_t id);

    VoxelDefEntryBuilder &type(VoxelType type);
    VoxelDefFaceBuilder face(VoxelFace face);
    VoxelDefFaceBuilder face(VoxelFace copy, VoxelFace face);

    void submit();

private:
    VoxelDefEntry entry;
    VoxelDef *owner;
    voxel_t id;
};

class VoxelDefFaceBuilder final {
public:
    VoxelDefFaceBuilder(VoxelDefEntryBuilder *parent, VoxelFace face, const VoxelDefEntry::Face &entry = VoxelDefEntry::Face());

    VoxelDefFaceBuilder &transparent(bool flag);
    VoxelDefFaceBuilder &texture(const std::string &path);
    VoxelDefFaceBuilder &clearTextures();

    VoxelDefEntryBuilder &endFace();

private:
    VoxelDefEntry::Face entry;
    VoxelDefEntryBuilder *parent;
    VoxelFace face;
};
} // namespace detail

class VoxelDef final {
public:
    friend class detail::VoxelDefEntryBuilder;

public:
    using map_type = std::unordered_map<voxel_t, VoxelDefEntry>;
    using const_iterator = map_type::const_iterator;

public:
    void clear();
    const VoxelDefEntry *find(voxel_t id) const;
    detail::VoxelDefEntryBuilder build(voxel_t id);
    size_t approximateTextureAmount() const;

    inline uint64_t getChecksum() const
    {
        return checksum;
    }

    inline const_iterator cbegin() const
    {
        return voxels.cbegin();
    }

    inline const_iterator cend() const
    {
        return voxels.cend();
    }

private:
    uint64_t checksum { 0 };
    map_type voxels;
};
