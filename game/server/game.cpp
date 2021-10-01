/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <ctime>
#include <game/server/chunks.hpp>
#include <game/server/game.hpp>
#include <game/server/globals.hpp>
#include <game/server/session_manager.hpp>
#include <game/shared/comp/chunk.hpp>
#include <game/shared/comp/creature.hpp>
#include <game/shared/comp/head.hpp>
#include <game/shared/comp/player.hpp>
#include <game/shared/protocol/packets/client/handshake.hpp>
#include <game/shared/protocol/packets/client/login_start.hpp>
#include <game/shared/protocol/packets/client/request_login_chunks.hpp>
#include <game/shared/protocol/packets/client/request_spawn.hpp>
#include <game/shared/protocol/packets/client/request_voxeldef.hpp>
#include <game/shared/protocol/packets/server/chunk_data.hpp>
#include <game/shared/protocol/packets/server/login_chunks_end.hpp>
#include <game/shared/protocol/packets/server/login_success.hpp>
#include <game/shared/protocol/packets/server/spawn_player.hpp>
#include <game/shared/protocol/packets/server/voxeldef_end.hpp>
#include <game/shared/protocol/packets/server/voxeldef_face.hpp>
#include <game/shared/protocol/packets/server/voxeldef_voxel.hpp>
#include <game/shared/protocol/packets/shared/disconnect.hpp>
#include <game/shared/util/enet.hpp>
#include <game/shared/voxels.hpp>
#include <glm/gtc/noise.hpp>
#include <spdlog/spdlog.h>
#include <random>
#include <unordered_map>

using packet_handler_t = void(*)(const std::vector<uint8_t> &, Session *);
static const std::unordered_map<uint16_t, packet_handler_t> packet_handlers = {
    {
        protocol::packets::Handshake::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
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
    },
    {
        protocol::packets::LoginStart::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::LoginStart packet;
            protocol::deserialize(payload, packet);

            session->state = SessionState::SENDING_GAME_DATA;
            session->username = packet.username;

            protocol::packets::LoginSuccess response = {};
            response.session_id = session->session_id;
            util::sendPacket(session->peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

            spdlog::info("Client {} logged in as {}", session->session_id, session->username);
        }
    },
    {
        protocol::packets::RequestVoxelDef::id,
        [](const std::vector<uint8_t> &, Session *session) {
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                protocol::packets::VoxelDefVoxel voxeldef_voxel = {};
                voxeldef_voxel.voxel = it->first;
                voxeldef_voxel.type = it->second.type;
                util::sendPacket(session->peer, voxeldef_voxel, 0, ENET_PACKET_FLAG_RELIABLE);

                for(const VoxelFaceInfo &face : it->second.faces) {
                    protocol::packets::VoxelDefFace voxeldef_face = {};
                    voxeldef_face.voxel = it->first;
                    voxeldef_face.face = face.face;
                    voxeldef_face.transparent = (it->second.transparency.find(face.face) != it->second.transparency.cend());
                    voxeldef_face.texture = face.texture;
                    util::sendPacket(session->peer, voxeldef_face, 0, ENET_PACKET_FLAG_RELIABLE);
                }
            }
            
            protocol::packets::VoxelDefEnd voxeldef_end = {};
            voxeldef_end.checksum = globals::voxels.getChecksum();
            util::sendPacket(session->peer, voxeldef_end, 0, ENET_PACKET_FLAG_RELIABLE);
        }
    },
    {
        protocol::packets::RequestLoginChunks::id,
        [](const std::vector<uint8_t> &, Session *session) {
            const auto view = globals::registry.view<ChunkComponent>();
            for(const auto [entity, chunk] : view.each()) {
                if(ServerChunk *chunkp = globals::chunks.find(chunk.position)) {
                    protocol::packets::ChunkData chunk_data;
                    math::vecToArray(chunk.position, chunk_data.position);
                    chunk_data.data = chunkp->data;
                    util::sendPacket(session->peer, chunk_data, 0, ENET_PACKET_FLAG_RELIABLE);
                }
            }

            util::sendPacket(session->peer, protocol::packets::LoginChunksEnd {}, 0, ENET_PACKET_FLAG_RELIABLE);
        }
    },
    {
        protocol::packets::RequestSpawn::id,
        [](const std::vector<uint8_t> &, Session *session) {
            session->player = globals::registry.create();

            CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(session->player);
            creature.position = FLOAT3_ZERO;
            creature.orientation = FLOATQUAT_IDENTITY;

            HeadComponent &head = globals::registry.emplace<HeadComponent>(session->player);
            head.angles = FLOAT2_ZERO;
            head.offset = FLOAT3_ZERO;

            protocol::packets::SpawnPlayer response;
            response.session_id = session->session_id;
            math::vecToArray(creature.position, response.position);
            math::vecToArray(head.angles, response.head_angles);
            util::sendPacket(session->peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

            session->state = SessionState::PLAYING;
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);
            spdlog::info("Client {} ({}) disconnected ({})", session->username, session->session_id, packet.reason);
            if(session->player != entt::null)
                globals::registry.destroy(session->player);
            enet_peer_disconnect(session->peer, 0);
        }
    }
};

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
            if(solidity > 0.0f) {
                int64_t h1 = ((solidity - 0.0f) * 32.0f);
                int64_t h2 = (hmod * 16.0f);
                for(int64_t vy = 1; vy < h1; vy++)
                    globals::chunks.set(voxelpos_t(vx, -vy, vz), 0x01, true);
                for(int64_t vy = 0; h1 && vy < h2; vy++)
                    globals::chunks.set(voxelpos_t(vx, vy, vz), (vy == h2 - 1) ? 0x03 : 0x02, VOXEL_SET_FORCE);
            }
        }
    }
}

void sv_game::init()
{
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

            uint16_t type;
            std::vector<uint8_t> payload;
            if(!protocol::split(pbuf, type, payload)) {
                spdlog::warn("Invalid packet format received from client {}", session->session_id);
                continue;
            }

            const auto it = packet_handlers.find(type);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} received from client {}", type, session->session_id);
                continue;
            }

            it->second(payload, session);
        }
    }
}
