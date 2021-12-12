/*
 * session.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <enet/enet.h>
#include <entt/entt.hpp>
#include <shared/voxels.hpp>
#include <shared/world.hpp>
#include <string>
#include <deque>
#include <unordered_set>

enum class SessionState {
    DISCONNECTED,
    CONNECTED,
    LOGGING_IN,
    RECEIVING_GAMEDATA,
    PLAYING
};

enum class ServerGameDataState {
    VOXEL_DEF,
    CHUNK_DATA,
    PLAYER_INFO,
    ENTITIES
};

struct Session {
    uint32_t id { 0 };
    ENetPeer *peer { nullptr };
    SessionState state { SessionState::CONNECTED };
    std::string username { "unnamed" };
    entt::entity player_entity { entt::null };
};

struct ClientSession final : public Session {
    // server-side entity ID
    uint32_t player_entity_id { 0 };
};

struct ServerSession final : public Session {
    // When we disconnect we must reduce
    // the reference count of these chunks
    std::unordered_set<chunkpos_t> loaded_chunks;
};
