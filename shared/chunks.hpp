/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <shared/world.hpp>
#include <unordered_map>

using chunk_create_flags_t = uint16_t;
constexpr const chunk_create_flags_t CHUNK_CREATE_UPDATE_NEIGHBOURS = (1 << 0);

using voxel_set_flags_t = uint16_t;
constexpr const voxel_set_flags_t VOXEL_SET_FORCE = (1 << 0);
constexpr const voxel_set_flags_t VOXEL_SET_UPDATE_NEIGHBOURS = (1 << 1);

template<typename chunk_type, typename T>
class ChunkManager {
public:
    virtual ~ChunkManager() = default;

    void clear();
    void remove(const chunkpos_t &cp);

    chunk_type *find(const chunkpos_t &cp);
    chunk_type *create(const chunkpos_t &cp, chunk_create_flags_t flags);

    voxel_t get(const voxelpos_t &vp) const;
    voxel_t get(const chunkpos_t &cp, const localpos_t &lp) const;
    bool set(const voxelpos_t &vp, voxel_t voxel, voxel_set_flags_t flags);

    // Implementations define:
    //  void implOnClear();
    //  void implOnRemove(const chunkpos_t &, const chunk_type &);
    //  chunk_type implOnCreate(const chunkpos_t &, chunk_create_flags_t);
    //  voxel_t implGetVoxel(const chunk_type &, const localpos_t &) const;
    //  void implSetVoxel(chunk_type *, const chunkpos_t &, const localpos_t &, voxel_t, voxel_set_flags_t);

protected:
    std::unordered_map<chunkpos_t, chunk_type> chunks;
};

template<typename chunk_type, typename T>
inline void ChunkManager<chunk_type, T>::clear()
{
    static_cast<T *>(this)->implOnClear();
    chunks.clear();
}

template<typename chunk_type, typename T>
inline void ChunkManager<chunk_type, T>::remove(const chunkpos_t &cp)
{
    const auto it = chunks.find(cp);
    if(it != chunks.cend()) {
        static_cast<T *>(this)->implOnRemove(cp, it->second);
        chunks.erase(it);
    }
}

template<typename chunk_type, typename T>
inline chunk_type *ChunkManager<chunk_type, T>::find(const chunkpos_t &cp)
{
    auto it = chunks.find(cp);
    if(it != chunks.end())
        return &it->second;
    return nullptr;
}

template<typename chunk_type, typename T>
inline chunk_type *ChunkManager<chunk_type, T>::create(const chunkpos_t &cp, chunk_create_flags_t flags)
{
    auto it = chunks.find(cp);
    if(it != chunks.end())
        return &it->second;
    return &(chunks[cp] = static_cast<T *>(this)->implOnCreate(cp, flags));
}

template<typename chunk_type, typename T>
inline voxel_t ChunkManager<chunk_type, T>::get(const voxelpos_t &vp) const
{
    const auto it = chunks.find(toChunkPos(vp));
    if(it != chunks.cend())
        return static_cast<const T *>(this)->implGetVoxel(it->second, toLocalPos(vp));
    return NULL_VOXEL;
}

template<typename chunk_type, typename T>
inline voxel_t ChunkManager<chunk_type, T>::get(const chunkpos_t &cp, const localpos_t &lp) const
{
    // This will patch negative localpos_t values.
    return get(toVoxelPos(cp, lp));
}

template<typename chunk_type, typename T>
inline bool ChunkManager<chunk_type, T>::set(const voxelpos_t &vp, voxel_t voxel, voxel_set_flags_t flags)
{
    const chunkpos_t cp = toChunkPos(vp);
    const localpos_t lp = toLocalPos(vp);
    chunk_type *chunk = find(cp);
    if(!chunk) {
        if(!(flags & VOXEL_SET_FORCE))
            return false;
        chunk = create(cp, 0);
    }

    static_cast<T *>(this)->implSetVoxel(chunk, cp, lp, voxel, flags);
    return true;
}

