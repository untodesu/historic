/*
 * chunks.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <shared/chunks.hpp>

struct ServerChunk final {
    entt::entity entity;
    voxel_array_t data;
    int refcount;
};

class ServerChunkManager final : public ChunkManager<ServerChunk, ServerChunkManager> {
public:
    void implOnClear();
    bool implOnRemove(const chunkpos_t &cp, ServerChunk &data);
    ServerChunk implOnCreate(const chunkpos_t &cp, voxel_set_flags_t flags);
    voxel_t implGetVoxel(const ServerChunk &data, const localpos_t &lp) const;
    void implSetVoxel(ServerChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags);
};
