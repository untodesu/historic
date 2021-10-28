/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <ctime>
#include <server/chunks.hpp>
#include <server/game.hpp>
#include <server/globals.hpp>
#include <server/session_manager.hpp>
#include <shared/comp/chunk.hpp>
#include <shared/comp/creature.hpp>
#include <shared/comp/head.hpp>
#include <shared/comp/player.hpp>
#include <shared/protocol/packets/client/handshake.hpp>
#include <shared/protocol/packets/client/login_start.hpp>
#include <shared/protocol/packets/client/request_gamedata.hpp>
#include <shared/protocol/packets/client/request_respawn.hpp>
#include <shared/protocol/packets/server/attach_creature.hpp>
#include <shared/protocol/packets/server/attach_head.hpp>
#include <shared/protocol/packets/server/attach_player.hpp>
#include <shared/protocol/packets/server/create_entity.hpp>
#include <shared/protocol/packets/server/gamedata_chunk_voxels.hpp>
#include <shared/protocol/packets/server/gamedata_end_request.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_entry.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_face.hpp>
#include <shared/protocol/packets/server/login_success.hpp>
#include <shared/protocol/packets/shared/disconnect.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <glm/gtc/noise.hpp>
#include <spdlog/spdlog.h>
#include <random>
#include <unordered_map>

static inline float octanoise(const float3 &v, unsigned int oct)
{
    float result = 1.0;
    for(unsigned int i = 1; i <= oct; i++)
        result += glm::simplex(v * static_cast<float>(i));
    return result / static_cast<float>(oct);
}

static void generate(uint64_t seed = 0)
{
    constexpr const int64_t START = -128;
    constexpr const int64_t END = 128;

    std::mt19937_64 mtgen = std::mt19937_64(seed);
    const float seed_f = std::uniform_real_distribution<float>()(mtgen);
    for(int64_t vx = START; vx < END; vx++) {
        for(int64_t vz = START; vz < END; vz++) {
            const float3 vxz = float3(vx, vz, seed_f * 5120.0f);
            const float solidity = octanoise(vxz / 160.0f, 3);
            const float hmod = octanoise((vxz + 1.0f) / 160.0f, 8);
            if(solidity >= 0.2f) {
                int64_t h1 = ((solidity - 0.2f) * 32.0f);
                int64_t h2 = (hmod * 16.0f);
                for(int64_t vy = 1; vy < h1; vy++)
                    globals::chunks.set(voxelpos_t(vx, -vy, vz), 0x01, true);
                for(int64_t vy = 0; h1 && vy < h2; vy++)
                    globals::chunks.set(voxelpos_t(vx, vy, vz), (vy == h2 - 1) ? 0x03 : 0x02, VOXEL_SET_FORCE);
            }
        }
    }
}

static void onHandshake(const std::vector<uint8_t> &payload, Session *session)
{
    protocol::packets::Handshake packet;
    protocol::deserialize(payload, packet);

    if(packet.version != protocol::VERSION) {
        protocol::packets::Disconnect response = {};
        response.reason = "Protocol versions do not match!";
        util::sendPacket(session->peer, response, 0, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_disconnect_later(session->peer, 0);
        return;
    }

    session->state = SessionState::LOGGING_IN;
}

static void onLoginStart(const std::vector<uint8_t> &payload, Session *session)
{
    protocol::packets::LoginStart packet;
    protocol::deserialize(payload, packet);

    session->state = SessionState::SENDING_GAME_DATA;
    session->username = packet.username;

    protocol::packets::LoginSuccess response = {};
    response.session_id = session->session_id;
    util::sendPacket(session->peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

    spdlog::info("Client {} logged in as {}", session->session_id, session->username);
}

static void onRequestGamedata(const std::vector<uint8_t> &, Session *session)
{
    for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
        protocol::packets::GamedataVoxelEntry entryp = {};
        entryp.voxel = it->first;
        entryp.type = it->second.type;
        util::sendPacket(session->peer, entryp, 0, 0);

        for(const VoxelFaceInfo &face : it->second.faces) {
            protocol::packets::GamedataVoxelFace facep = {};
            facep.voxel = it->first;
            facep.face = face.face;
            facep.transparent = (it->second.transparency.find(face.face) != it->second.transparency.cend()) ? 1 : 0;
            facep.texture = face.texture;
            util::sendPacket(session->peer, facep, 0, 0);
        }
    }

    const auto view = globals::registry.view<ChunkComponent>();
    for(const auto [entity, chunk] : view.each()) {
        if(const ServerChunk *pchunk = globals::chunks.find(chunk.position)) {
            protocol::packets::GamedataChunkVoxels chunkp = {};
            math::vecToArray(chunk.position, chunkp.position);
            chunkp.data = pchunk->data;
            util::sendPacket(session->peer, chunkp, 0, 0);
        }
    }

    protocol::packets::GamedataEndRequest endp = {};
    endp.voxel_checksum = globals::voxels.getChecksum();
    util::sendPacket(session->peer, endp, 0, 0);
}

static void onRequestRespawn(const std::vector<uint8_t> &, Session *session)
{
    session->player = globals::registry.create();

    CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(session->player);
    creature.position = FLOAT3_ZERO;
    creature.yaw = 0.0f;

    HeadComponent &head = globals::registry.emplace<HeadComponent>(session->player);
    head.angles = FLOAT2_ZERO;
    head.offset = FLOAT3_ZERO;

    protocol::packets::CreateEntity createp = {};
    createp.network_id = (uint32_t)session->player;
    util::sendPacket(session->peer, createp, 0, 0);

    protocol::packets::AttachCreature creaturep = {};
    creaturep.network_id = (uint32_t)session->player;
    math::vecToArray(creature.position, creaturep.position);
    creaturep.yaw = creature.yaw;
    util::sendPacket(session->peer, creaturep, 0, 0);

    protocol::packets::AttachHead headp = {};
    headp.network_id = (uint32_t)session->player;
    math::vecToArray(head.angles, headp.angles);
    math::vecToArray(head.offset, headp.offset);
    util::sendPacket(session->peer, headp, 0, 0);

    protocol::packets::AttachPlayer playerp = {};
    playerp.network_id = (uint32_t)session->player;
    playerp.session_id = session->session_id;
    util::sendPacket(session->peer, playerp, 0, 0);
}

static void onDisconnect(const std::vector<uint8_t> &payload, Session *session)
{
    protocol::packets::Disconnect packet;
    protocol::deserialize(payload, packet);
    spdlog::info("Client {} ({}) disconnected ({})", session->username, session->session_id, packet.reason);
    if(session->player != entt::null)
        globals::registry.destroy(session->player);
    enet_peer_disconnect(session->peer, 0);
}

void sv_game::init()
{
    globals::packet_handlers[protocol::packets::Handshake::id] = &onHandshake;
    globals::packet_handlers[protocol::packets::LoginStart::id] = &onLoginStart;
    globals::packet_handlers[protocol::packets::RequestGamedata::id] = &onRequestGamedata;
    globals::packet_handlers[protocol::packets::RequestRespawn::id] = &onRequestRespawn;
    globals::packet_handlers[protocol::packets::Disconnect::id] = &onDisconnect;

    session_manager::init();
}

void sv_game::postInit()
{
    // Stone
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/stone.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/stone.png" });
        globals::voxels.set(0x01, vinfo);
    }

    // Dirt
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/dirt.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/dirt.png" });
        globals::voxels.set(0x02, vinfo);
    }

    // Grass
    {
        VoxelInfo vinfo = {};
        vinfo.type = VoxelType::SOLID;
        vinfo.faces.push_back({ VoxelFace::LF, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::RT, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::FT, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::BK, "textures/grass_side.png" });
        vinfo.faces.push_back({ VoxelFace::UP, "textures/grass.png" });
        vinfo.faces.push_back({ VoxelFace::DN, "textures/dirt.png" });
        globals::voxels.set(0x03, vinfo);
    }

    uint64_t seed = static_cast<uint64_t>(std::time(nullptr));
    spdlog::info("Generating ({})...", seed);
    generate(seed);
    spdlog::info("Generating done");
}

void sv_game::shutdown()
{
    session_manager::kickAll("Server is shutting down");
    globals::registry.clear();
}

void sv_game::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            Session *session = session_manager::create();
            session->peer = event.peer;
            session->state = SessionState::CONNECTED;
            event.peer->data = session;
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
            session_manager::destroy(reinterpret_cast<Session *>(event.peer->data));
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            Session *session = reinterpret_cast<Session *>(event.peer->data);

            const std::vector<uint8_t> pbuf = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t packet_id;
            std::vector<uint8_t> payload;
            if(!protocol::split(pbuf, packet_id, payload)) {
                spdlog::warn("Invalid packet format received from client {}", session->session_id);
                continue;
            }

            const auto it = globals::packet_handlers.find(packet_id);
            if(it == globals::packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} received from client {}", packet_id, session->session_id);
                continue;
            }

            it->second(payload, session);
        }
    }
}
