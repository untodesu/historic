/*
 * chunks.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <entt/entt.hpp>
#include <shared/chunks.hpp>
#include <shared/config.hpp>
#include <server/vgen.hpp>

struct ServerChunk final {
    entt::entity entity;
    voxel_array_t data;
    int refcount;
};

class WorldConfig final : public BaseConfig<WorldConfig> {
public:
    void implPostRead();
    void implPreWrite();

public:
    int32_t base;
    int32_t height;
    struct {
        uint64_t seed;
    } generator;
};

class ServerChunkManager final : public ChunkManager<ServerChunk, ServerChunkManager> {
public:
    void implOnClear();
    bool implOnRemove(const chunkpos_t &cp, ServerChunk &data);
    ServerChunk implOnCreate(const chunkpos_t &cp, voxel_set_flags_t flags);
    voxel_t implGetVoxel(const ServerChunk &data, const localpos_t &lp) const;
    void implSetVoxel(ServerChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags);

    void init();
    void shutdown();
    ServerChunk *load(const chunkpos_t &cp);
    void free(const chunkpos_t &cp);

public:
    WorldConfig config;

private:
    VGen vgen;
};
