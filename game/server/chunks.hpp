/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <entt/entt.hpp>
#include <game/shared/chunks.hpp>

struct ServerChunk final {
    entt::entity entity;
    voxel_array_t data;
};

class ServerChunkManager final : public ChunkManager<ServerChunk, ServerChunkManager> {
public:
    void implOnClear();
    void implOnRemove(const chunkpos_t &cp, const ServerChunk &data);
    ServerChunk implOnCreate(const chunkpos_t &cp);
    voxel_t implGetVoxel(const ServerChunk &data, const localpos_t &lp) const;
    void implSetVoxel(ServerChunk *data, const chunkpos_t &cp, const localpos_t &lp, voxel_t voxel, voxel_set_flags_t flags);
};
