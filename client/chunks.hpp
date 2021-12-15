/*
 * chunks.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <shared/chunks.hpp>

struct ClientChunk final {
    entt::entity entity;
    voxel_array_t data;
};

class ClientChunkManager final : public ChunkManager<ClientChunk, ClientChunkManager> {
public:
    void implOnClear();
    bool implOnRemove(const chunkpos_t &cp, const ClientChunk &data);
    ClientChunk implOnCreate(const chunkpos_t &cp);
    voxel_t implGetVoxel(const ClientChunk &data, const localpos_t &lp) const;
    void implSetVoxel(ClientChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags);
};
