/*
 * voxels.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>
#include <shared/world.hpp>
#include <unordered_map>
#include <unordered_set>

using voxel_type_t = uint8_t;
constexpr static const voxel_type_t VOXEL_NULL_TYPE = 0x00;
constexpr static const voxel_type_t VOXEL_GAS       = 0x01;
constexpr static const voxel_type_t VOXEL_SOLID     = 0x02;
constexpr static const voxel_type_t VOXEL_LIQUID    = 0x03;

using voxel_face_t = uint8_t;
constexpr static const voxel_face_t VOXEL_FACE_LF   = 0x01;
constexpr static const voxel_face_t VOXEL_FACE_FT   = 0x02;
constexpr static const voxel_face_t VOXEL_FACE_DN   = 0x03;
constexpr static const voxel_face_t VOXEL_FACE_RT   = 0x10;
constexpr static const voxel_face_t VOXEL_FACE_BK   = 0x20;
constexpr static const voxel_face_t VOXEL_FACE_UP   = 0x30;

// The game should support up to 16 different voxel faces
// with each capable of being inverted. Thus we can
// have a single byte to represent any possible face
// with most significant bits for regular faces and
// least significant bits for inverted faces. A face
// index that contains both parts is either reserved or
// a face that can't be inverted (like those diagonal
// faces for plants or whatever I'd add in future).
constexpr static const voxel_face_t VOXEL_FACE_MASK_REG = 0xF0;
constexpr static const voxel_face_t VOXEL_FACE_MASK_INV = 0x0F;

constexpr static inline const voxel_face_t flipVoxelFace(const voxel_face_t face)
{
    if(face & VOXEL_FACE_MASK_REG)
        return (face >> 4) & VOXEL_FACE_MASK_INV;
    return (face << 4) & VOXEL_FACE_MASK_REG;
}

constexpr static inline const float voxelFaceNormal(const voxel_face_t face)
{
    if(face & VOXEL_FACE_MASK_INV)
        return 1.0f;
    if(face & VOXEL_FACE_MASK_REG)
        return -1.0f;
    return 0.0f;
}

struct VoxelDefEntry final {
    struct Face final {
        bool transparent { false };
        std::string texture;
    };

    voxel_type_t type { VOXEL_NULL_TYPE };
    std::unordered_map<voxel_face_t, Face> faces;
};

class VoxelDef final {
public:
    class FaceBuilder;
    class EntryBuilder final {
        friend class FaceBuilder;

    public:
        EntryBuilder(VoxelDef *owner, voxel_t id);
        EntryBuilder(const EntryBuilder &rhs) = delete;
        EntryBuilder(EntryBuilder &&rhs) = delete;

        EntryBuilder &operator=(const EntryBuilder &rhs) = delete;
        EntryBuilder &operator=(EntryBuilder &&rhs) = delete;

        EntryBuilder &type(voxel_type_t type);
        FaceBuilder face(voxel_face_t face);
        FaceBuilder face(voxel_face_t copy, voxel_face_t face);

        void submit();

    private:
        VoxelDefEntry entry;
        VoxelDef *owner;
        voxel_t id;
    };

    class FaceBuilder final {
    public:
        FaceBuilder(EntryBuilder *parent, voxel_face_t face, const VoxelDefEntry::Face &entry = VoxelDefEntry::Face());
        FaceBuilder(const FaceBuilder &rhs) = delete;
        FaceBuilder(FaceBuilder &&rhs) = delete;

        FaceBuilder &operator=(const FaceBuilder &rhs) = delete;
        FaceBuilder &operator=(FaceBuilder &&rhs) = delete;

        FaceBuilder &transparent(bool flag);
        FaceBuilder &texture(const std::string &path);

        EntryBuilder &endFace();

    private:
        VoxelDefEntry::Face entry;
        EntryBuilder *parent;
        voxel_face_t face;
    };

public:
    using map_type = std::unordered_map<voxel_t, VoxelDefEntry>;
    using const_iterator = map_type::const_iterator;

public:
    void clear();
    const VoxelDefEntry *find(voxel_t id) const;
    EntryBuilder build(voxel_t id);

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


#if 0
class VoxelDef final {
public:
    using map_type = std::unordered_map<voxel_t, VoxelInfo>;
    using const_iterator = map_type::const_iterator;

public:
    VoxelDef();

    void clear();
    bool set(voxel_t voxel, const VoxelInfo &info);
    const VoxelInfo *tryGet(voxel_t voxel) const;

    inline uint64_t getChecksum() const
    {
        return checksum;
    }

    inline const_iterator cbegin() const
    {
        return def.cbegin();
    }

    inline const_iterator cend() const
    {
        return def.cend();
    }

private:
    uint64_t checksum;
    map_type def;
};
#endif
