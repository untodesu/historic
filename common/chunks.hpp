/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/world.hpp>
#include <unordered_map>

using voxel_set_flags_t = uint16_t;
constexpr const voxel_set_flags_t VOXEL_SET_FORCE = (1 << 0);
constexpr const voxel_set_flags_t VOXEL_SET_UPDATE_NEIGHBOURS = (1 << 1);

template<typename chunk_type, typename impl_type>
class ChunkManager {
public:
    virtual ~ChunkManager() = default;

    void clear();
    void remove(const chunk_pos_t &cpos);

    chunk_type *find(const chunk_pos_t &cpos);
    chunk_type *create(const chunk_pos_t &cpos);

    voxel_t getVoxel(const voxel_pos_t &vpos) const;
    voxel_t getVoxel(const chunk_pos_t &cpos, const local_pos_t &lpos) const;

    bool trySetVoxel(const voxel_pos_t &vpos, voxel_t voxel, voxel_set_flags_t flags = 0);

    // Implementations define:
    //  void impl_onClear();
    //  bool impl_onRemove(const chunk_pos_t &, const chunk_type &);
    //  chunk_type impl_onCreate(const chunk_pos_t &);
    //  voxel_t impl_onGetVoxel(const chunk_type &, const local_pos_t &) const;
    //  void impl_onSetVoxel(chunk_type *, const chunk_pos_t &, const local_pos_t &, voxel_t, voxel_set_flags_t);

protected:
    std::unordered_map<chunk_pos_t, chunk_type> chunks;
};

template<typename chunk_type, typename impl_type>
inline void ChunkManager<chunk_type, impl_type>::clear()
{
    static_cast<impl_type *>(this)->impl_onClear();
    chunks.clear();
}

template<typename chunk_type, typename impl_type>
inline void ChunkManager<chunk_type, impl_type>::remove(const chunk_pos_t &cpos)
{
    const auto it = chunks.find(cpos);
    if(it == chunks.cend() || static_cast<impl_type *>(this)->impl_onRemove(cpos, it->second))
        return;
    chunks.erase(it);
}

template<typename chunk_type, typename impl_type>
inline chunk_type *ChunkManager<chunk_type, impl_type>::find(const chunk_pos_t &cpos)
{
    auto it = chunks.find(cpos);
    if(it != chunks.end())
        return &it->second;
    return nullptr;
}

template<typename chunk_type, typename impl_type>
inline chunk_type *ChunkManager<chunk_type, impl_type>::create(const chunk_pos_t &cpos)
{
    auto it = chunks.find(cpos);
    if(it != chunks.end())
        &it->second;
    return &(chunks[cpos] = static_cast<impl_type *>(this)->impl_onCreate(cpos));
}

template<typename chunk_type, typename impl_type>
inline voxel_t ChunkManager<chunk_type, impl_type>::getVoxel(const voxel_pos_t &vpos) const
{
    const auto it = chunks.find(world::getChunkPosition(vpos));
    if(it != chunks.cend())
        return static_cast<const impl_type *>(this)->impl_onGetVoxel(it->second, world::getLocalPosition(vpos));
    return NULL_VOXEL;
}

template<typename chunk_type, typename impl_type>
inline voxel_t ChunkManager<chunk_type, impl_type>::getVoxel(const chunk_pos_t &cpos, const local_pos_t &lpos) const
{
    return get(world::getVoxelPosition(cpos, lpos));
}

template<typename chunk_type, typename impl_type>
inline bool ChunkManager<chunk_type, impl_type>::trySetVoxel(const voxel_pos_t &vpos, voxel_t voxel, voxel_set_flags_t flags)
{
    const chunk_pos_t cpos = world::getChunkPosition(vpos);
    const local_pos_t lpos = world::getLocalPosition(vpos);

    chunk_type *chunk = find(cpos);
    if(!chunk) {
        if(!(flags & VOXEL_SET_FORCE))
            return false;
        chunk = create(cpos);
    }

    static_cast<impl_type *>(this)->impl_onSetVoxel(chunk, cpos, lpos, voxel, flags);

    return true;
}
