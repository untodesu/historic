/*
 * voxels.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <shared/voxels.hpp>

detail::VoxelDefEntryBuilder::VoxelDefEntryBuilder(VoxelDef *owner, voxel_t id)
    : entry(), owner(owner), id(id)
{

}

detail::VoxelDefEntryBuilder &detail::VoxelDefEntryBuilder::type(voxel_type_t type)
{
    entry.type = type;
    return *this;
}

detail::VoxelDefFaceBuilder detail::VoxelDefEntryBuilder::face(voxel_face_t face)
{
    return detail::VoxelDefFaceBuilder(this, face);
}

detail::VoxelDefFaceBuilder detail::VoxelDefEntryBuilder::face(voxel_face_t copy, voxel_face_t face)
{
    const auto it = entry.faces.find(copy);
    if(it != entry.faces.cend())
        return detail::VoxelDefFaceBuilder(this, face, it->second);
    return detail::VoxelDefFaceBuilder(this, face);
}

void detail::VoxelDefEntryBuilder::submit()
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

detail::VoxelDefFaceBuilder::VoxelDefFaceBuilder(detail::VoxelDefEntryBuilder *parent, voxel_face_t face, const VoxelDefEntry::Face &entry)
    : entry(entry), parent(parent), face(face)
{

}

detail::VoxelDefFaceBuilder &detail::VoxelDefFaceBuilder::transparent(bool flag)
{
    entry.transparent = flag;
    return *this;
}

detail::VoxelDefFaceBuilder &detail::VoxelDefFaceBuilder::texture(const std::string &path)
{
    entry.texture = path;
    return *this;
}

detail::VoxelDefEntryBuilder &detail::VoxelDefFaceBuilder::endFace()
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

detail::VoxelDefEntryBuilder VoxelDef::build(voxel_t id)
{
    return detail::VoxelDefEntryBuilder(this, id);
}
