/*
 * network.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <enet/enet.h>
#include <exception>
#include <server/chunks.hpp>
#include <server/globals.hpp>
#include <server/network.hpp>
#include <server/session_manager.hpp>
#include <shared/components/chunk.hpp>
#include <shared/components/creature.hpp>
#include <shared/components/head.hpp>
#include <shared/components/player.hpp>
#include <shared/protocol/packets/client/handshake.hpp>
#include <shared/protocol/packets/client/login_start.hpp>
#include <shared/protocol/packets/client/request_gamedata.hpp>
#include <shared/protocol/packets/client/request_respawn.hpp>
#include <shared/protocol/packets/server/gamedata_chunk_voxels.hpp>
#include <shared/protocol/packets/server/gamedata_end_request.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_entry.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_face.hpp>
#include <shared/protocol/packets/server/login_success.hpp>
#include <shared/protocol/packets/server/spawn_player.hpp>
#include <shared/protocol/packets/shared/creature_position.hpp>
#include <shared/protocol/packets/shared/disconnect.hpp>
#include <shared/protocol/packets/shared/head_angles.hpp>
#include <shared/protocol/protocol.hpp>
#include <shared/util/enet.hpp>
#include <spdlog/spdlog.h>

static const std::unordered_map<uint16_t, void(*)(const std::vector<uint8_t> &, Session *)> packet_handlers = {
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

            session->state = SessionState::LOADING_GAMEDATA;
            session->username = packet.username;

            protocol::packets::LoginSuccess response = {};
            response.session_id = session->id;
            util::sendPacket(session->peer, response, 0, ENET_PACKET_FLAG_RELIABLE);

            spdlog::info("Client {} logged in as {}", session->id, session->username);
        }
    },
    {
        protocol::packets::RequestGamedata::id,
        [](const std::vector<uint8_t> &, Session *session) {
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
    },
    {
        protocol::packets::RequestRespawn::id,
        [](const std::vector<uint8_t> &, Session *session) {
            session->player_entity = globals::registry.create();
            
            protocol::packets::SpawnPlayer spawnp = {};
            spawnp.session_id = session->id;
            spawnp.network_id = static_cast<uint32_t>(session->player_entity);

            HeadComponent &head = globals::registry.emplace<HeadComponent>(session->player_entity);
            head.angles = FLOAT2_ZERO;
            head.offset = FLOAT3_ZERO;
            math::vecToArray(head.angles, spawnp.head_angles);

            CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(session->player_entity);
            creature.position = FLOAT3_ZERO;
            creature.yaw = 0.0f;
            math::vecToArray(creature.position, spawnp.position);
            spawnp.yaw = creature.yaw;
            
            util::broadcastPacket(globals::host, spawnp, 0, 0);
            session->state = SessionState::PLAYING;
        }
    },
    {
        protocol::packets::CreaturePosition::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::CreaturePosition packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = static_cast<entt::entity>(packet.network_id);
            if(globals::registry.valid(entity) && entity == session->player_entity) {
                if(CreatureComponent *creature = globals::registry.try_get<CreatureComponent>(entity)) {
                    creature->position = math::arrayToVec<float3>(packet.position);
                    util::broadcastPacket(globals::host, packet, 0, 0);
                }
            }
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);
            spdlog::info("Client {} ({}) disconnected ({})", session->username, session->id, packet.reason);
            if(globals::registry.valid(session->player_entity))
                globals::registry.destroy(session->player_entity);
            enet_peer_disconnect(session->peer, 0);
        }
    },
    {
        protocol::packets::HeadAngles::id,
        [](const std::vector<uint8_t> &payload, Session *session) {
            protocol::packets::HeadAngles packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = static_cast<entt::entity>(packet.network_id);
            if(globals::registry.valid(entity) && entity == session->player_entity) {
                if(HeadComponent *head = globals::registry.try_get<HeadComponent>(entity)) {
                    head->angles = math::arrayToVec<float3>(packet.angles);
                    util::broadcastPacket(globals::host, packet, 0, 0);
                }
            }
        }
    }
};


void sv_network::init()
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = protocol::DEFAULT_PORT;

    globals::host = enet_host_create(&address, 16, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a server host object.");
        std::terminate();
    }

    session_manager::init();
}

void sv_network::shutdown()
{
    session_manager::kickAll("Server shutting down.");
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

void sv_network::update()
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
                spdlog::warn("Invalid packet format received from client {}", session->id);
                continue;
            }

            const auto it = packet_handlers.find(packet_id);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X} from {}", packet_id, session->id);
                continue;
            }

            it->second(payload, session);
        }
    }
}
