/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/voxels.hpp>

detail::VoxelDefEntryBuilder::VoxelDefEntryBuilder(VoxelDef *owner, voxel_t id)
    : entry(), owner(owner), id(id)
{

}

detail::VoxelDefEntryBuilder &detail::VoxelDefEntryBuilder::type(VoxelType type)
{
    entry.type = type;
    return *this;
}

detail::VoxelDefFaceBuilder detail::VoxelDefEntryBuilder::face(VoxelFace face)
{
    return detail::VoxelDefFaceBuilder(this, face);
}

detail::VoxelDefFaceBuilder detail::VoxelDefEntryBuilder::face(VoxelFace copy, VoxelFace face)
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
        owner_entry->second = entry;
        return;
    }

    owner->voxels[id] = entry;
    owner->checksum += entry.faces.size();
}

detail::VoxelDefFaceBuilder::VoxelDefFaceBuilder(detail::VoxelDefEntryBuilder *parent, VoxelFace face, const VoxelDefEntry::Face &entry)
    : entry(entry), parent(parent), face(face)
{
    this->entry.transparent = false;
}

detail::VoxelDefFaceBuilder &detail::VoxelDefFaceBuilder::transparent(bool flag)
{
    entry.transparent = flag;
    return *this;
}

detail::VoxelDefFaceBuilder &detail::VoxelDefFaceBuilder::texture(const std::string &path)
{
    entry.textures.push_back(path);
    return *this;
}

detail::VoxelDefFaceBuilder &detail::VoxelDefFaceBuilder::clearTextures()
{
    entry.textures.clear();
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

size_t VoxelDef::approximateTextureAmount() const
{
    size_t result = 0;
    for(const auto def : voxels) {
        for(const auto def_face : def.second.faces) {
            for(const auto texture : def_face.second.textures) {
                result++;
            }
        }
    }
    return result;
}
