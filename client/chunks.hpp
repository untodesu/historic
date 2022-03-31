/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <common/chunks.hpp>
#include <entt/entt.hpp>

struct ClientChunk final {
    chunk_t data;
    entt::entity entity;
};

class ClientChunkManager final : public ChunkManager<ClientChunk, ClientChunkManager> {
public:
    void impl_onClear();
    bool impl_onRemove(const chunk_pos_t &cpos, const ClientChunk &chunk);
    ClientChunk impl_onCreate(const chunk_pos_t &cpos);
    voxel_t impl_onGetVoxel(const ClientChunk &cpos, const local_pos_t &lpos) const;
    void impl_onSetVoxel(ClientChunk *chunk, const chunk_pos_t &cpos, const local_pos_t &lpos, voxel_t, voxel_set_flags_t flags);
};

