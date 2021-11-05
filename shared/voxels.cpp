/*
 * voxels.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <shared/voxels.hpp>

VoxelDef::EntryBuilder::EntryBuilder(VoxelDef *owner, voxel_t id)
    : entry(), owner(owner), id(id)
{

}

VoxelDef::EntryBuilder &VoxelDef::EntryBuilder::type(voxel_type_t type)
{
    entry.type = type;
    return *this;
}

VoxelDef::FaceBuilder VoxelDef::EntryBuilder::face(voxel_face_t face)
{
    return VoxelDef::FaceBuilder(this, face);
}

VoxelDef::FaceBuilder VoxelDef::EntryBuilder::face(voxel_face_t copy, voxel_face_t face)
{
    const auto it = entry.faces.find(copy);
    if(it != entry.faces.cend())
        return VoxelDef::FaceBuilder(this, face, it->second);
    return VoxelDef::FaceBuilder(this, face);
}

void VoxelDef::EntryBuilder::submit()
{
    auto owner_entry = owner->voxels.find(id);
    if(owner_entry != owner->voxels.end()) {
        if(entry.type != VOXEL_NULL_TYPE)
            owner_entry->second.type = entry.type;
        owner->checksum -= owner_entry->second.faces.size();
        for(const auto my_face : entry.faces)
            owner_entry->second.faces[my_face.first] = my_face.second;
        owner->checksum += owner_entry->second.faces.size();
        return;
    }

    owner->voxels[id] = entry;
    owner->checksum += entry.faces.size();
}

VoxelDef::FaceBuilder::FaceBuilder(EntryBuilder *parent, voxel_face_t face, const VoxelDefEntry::Face &entry)
    : entry(entry), parent(parent), face(face)
{

}

VoxelDef::FaceBuilder &VoxelDef::FaceBuilder::transparent(bool flag)
{
    entry.transparent = flag;
    return *this;
}

VoxelDef::FaceBuilder &VoxelDef::FaceBuilder::texture(const std::string &path)
{
    entry.texture = path;
    return *this;
}

VoxelDef::EntryBuilder &VoxelDef::FaceBuilder::endFace()
{
    parent->entry.faces[face] = entry;
    return *parent;
}

void VoxelDef::clear()
{
    checksum = 0;
    voxels.clear();
}

const VoxelDefEntry *VoxelDef::find(voxel_t id) const
{
    const auto it = voxels.find(id);
    if(it != voxels.cend())
        return &it->second;
    return nullptr;
}

VoxelDef::EntryBuilder VoxelDef::build(voxel_t id)
{
    return VoxelDef::EntryBuilder(this, id);
}
