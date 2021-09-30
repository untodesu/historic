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
#include <game/server/session_info.hpp>
#include <game/shared/comp/chunk.hpp>
#include <game/shared/comp/creature.hpp>
#include <game/shared/comp/head.hpp>
#include <game/shared/comp/player.hpp>
#include <game/shared/protocol/packets/client/handshake.hpp>
#include <game/shared/protocol/packets/client/login_start.hpp>
#include <game/shared/protocol/packets/client/request_chunks.hpp>
#include <game/shared/protocol/packets/client/request_voxels.hpp>
#include <game/shared/protocol/packets/server/chunk_data.hpp>
#include <game/shared/protocol/packets/server/client_spawn.hpp>
#include <game/shared/protocol/packets/server/login_success.hpp>
#include <game/shared/protocol/packets/server/voxel_checksum.hpp>
#include <game/shared/protocol/packets/server/voxel_face_info.hpp>
#include <game/shared/protocol/packets/server/voxel_info.hpp>
#include <game/shared/protocol/packets/shared/disconnect.hpp>
#include <game/shared/protocol/enet.hpp>
#include <game/shared/voxels.hpp>
#include <glm/gtc/noise.hpp>
#include <spdlog/spdlog.h>
#include <random>
#include <unordered_map>

using packet_handler_t = void(*)(const std::vector<uint8_t> &, SessionInfo *, ENetPeer *);
static const std::unordered_map<uint16_t, packet_handler_t> packet_handlers = {
    {
        protocol::packets::Handshake::id,
        [](const std::vector<uint8_t> &payload, SessionInfo *info, ENetPeer *peer) {
            protocol::packets::Handshake packet;
            protocol::deserialize(payload, packet);

            if(packet.version != protocol::VERSION) {
                protocol::packets::Disconnect response = {};
                response.reason = "Protocol versions do not match!";
                protocol::send(peer, response, 0, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_disconnect_later(peer, 0);
                return;
            }

            info->state = protocol::SessionState::RECEIVE_LOGIN;
        }
    },
    {
        protocol::packets::LoginStart::id,
        [](const std::vector<uint8_t> &payload, SessionInfo *info, ENetPeer *peer) {
            protocol::packets::LoginStart packet;
            protocol::deserialize(payload, packet);

            info->state = protocol::SessionState::SEND_GAME_DATA;
            info->username = packet.username;

            protocol::packets::LoginSuccess response = {};
            response.session_id = info->session_id;
            protocol::send(peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

            spdlog::info("Client {} logged in as {}", info->session_id, info->username);
        }
    },
    {
        protocol::packets::RequestVoxels::id,
        [](const std::vector<uint8_t> &, SessionInfo *info, ENetPeer *peer) {
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                protocol::packets::VoxelInfo voxel_info = {};
                voxel_info.voxel = it->first;
                voxel_info.type = it->second.type;
                protocol::send(peer, voxel_info, 0, ENET_PACKET_FLAG_RELIABLE);

                for(const VoxelFaceInfo &face : it->second.faces) {
                    protocol::packets::VoxelFaceInfo voxel_face_info = {};
                    voxel_face_info.voxel = it->first;
                    voxel_face_info.face = face.face;
                    voxel_face_info.transparent = (it->second.transparency.find(face.face) != it->second.transparency.cend());
                    voxel_face_info.texture = face.texture;
                    protocol::send(peer, voxel_face_info, 0, ENET_PACKET_FLAG_RELIABLE);
                }
            }
            
            protocol::packets::VoxelChecksum checksum = {};
            checksum.checksum = globals::voxels.getChecksum();
            protocol::send(peer, checksum, 0, ENET_PACKET_FLAG_RELIABLE);
        }
    },
    {
        protocol::packets::RequestChunks::id,
        [](const std::vector<uint8_t> &, SessionInfo *info, ENetPeer *peer) {
            const auto view = globals::registry.view<ChunkComponent>();
            for(const auto [entity, chunk] : view.each()) {
                if(ServerChunk *chunkp = globals::chunks.find(chunk.position)) {
                    protocol::packets::ChunkData data;
                    math::vecToArray(chunk.position, data.position);
                    data.data = chunkp->data;
                    protocol::send(peer, data, 0, ENET_PACKET_FLAG_RELIABLE);
                }
            }

            // trollge
            {
                info->entity = globals::registry.create();

                CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(info->entity);
                creature.position = FLOAT3_ZERO;
                creature.orientation = FLOATQUAT_IDENTITY;

                HeadComponent &head = globals::registry.emplace<HeadComponent>(info->entity);
                head.angles = FLOAT3_ZERO;
                head.offset = FLOAT3_ZERO;

                protocol::packets::ClientSpawn response;
                math::vecToArray(creature.position, response.position);
                math::vecToArray(head.angles, response.head_angles);
                protocol::send(peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

                info->state = protocol::SessionState::PLAYING;
            }
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload, SessionInfo *info, ENetPeer *peer) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);
            spdlog::info("Client {} ({}) disconnected ({})", info->username, info->session_id, packet.reason);
            if(info->entity != entt::null)
                globals::registry.destroy(info->entity);
            enet_peer_disconnect(peer, 0);
        }
    }
};

static uint32_t getNewSessionID()
{
    static uint32_t counter = 0;
    return counter++;
}

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
            if(solidity > 0.1f) {
                int64_t h1 = ((solidity - 0.1f) * 32.0f);
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
    // We literally do nothing here at the moment
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
    globals::registry.clear();
}

void sv_game::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            SessionInfo *info = new SessionInfo;
            info->session_id = getNewSessionID();
            info->username = "unnamed";
            info->entity = entt::null;
            info->state = protocol::SessionState::RECEIVE_HANDSHAKE;
            event.peer->data = info;
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
            spdlog::info("Someone disconnected...");
            delete reinterpret_cast<SessionInfo *>(event.peer->data);
            continue;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            SessionInfo *info = reinterpret_cast<SessionInfo *>(event.peer->data);

            const std::vector<uint8_t> pbuf = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t type;
            std::vector<uint8_t> payload;
            if(!protocol::split(pbuf, type, payload)) {
                spdlog::warn("Invalid packet format received from client {}", info->session_id);
                continue;
            }

            const auto it = packet_handlers.find(type);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} received from client {}", type, info->session_id);
                continue;
            }

            it->second(payload, info, event.peer);
        }
    }

    // We literally do nothing here at the moment
}
