/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
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
#include <game/shared/protocol/packets/client/request_spawn.hpp>
#include <game/shared/protocol/packets/client/request_voxels.hpp>
#include <game/shared/protocol/packets/server/chunk_data.hpp>
#include <game/shared/protocol/packets/server/client_spawn.hpp>
#include <game/shared/protocol/packets/server/login_success.hpp>
#include <game/shared/protocol/packets/server/voxels_checksum.hpp>
#include <game/shared/protocol/packets/server/voxels_face.hpp>
#include <game/shared/protocol/packets/shared/disconnect.hpp>
#include <game/shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <unordered_map>

using packet_handler_t = void(*)(const std::vector<uint8_t> &, SessionInfo *, ENetPeer *);
static const std::unordered_map<uint16_t, packet_handler_t> packet_handlers = {
    {
        protocol::packets::Handshake::id,
        [](const std::vector<uint8_t> &payload, SessionInfo *info, ENetPeer *peer) {
            protocol::packets::Handshake packet;
            protocol::deserialize(payload, packet);

            if(packet.version != protocol::VERSION) {
                protocol::packets::Disconnect response;
                response.reason = "Protocol versions do not match!";
                const std::vector<uint8_t> rpbuf = protocol::serialize(response);
                enet_peer_send(peer, 0, enet_packet_create(rpbuf.data(), rpbuf.size(), ENET_PACKET_FLAG_RELIABLE));
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

            protocol::packets::LoginSuccess response;
            response.session_id = info->session_id;
            const std::vector<uint8_t> rpbuf = protocol::serialize(response);
            enet_peer_send(peer, 0, enet_packet_create(rpbuf.data(), rpbuf.size(), ENET_PACKET_FLAG_RELIABLE));

            spdlog::info("Client {} logged in as {}", info->session_id, info->username);
        }
    },
    {
        protocol::packets::RequestVoxels::id,
        [](const std::vector<uint8_t> &, SessionInfo *info, ENetPeer *peer) {
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                for(const VoxelFaceInfo &face : it->second.faces) {
                    protocol::packets::VoxelsFace packet;
                    packet.voxel = it->first;
                    packet.face = face.face;
                    packet.transparent = (it->second.transparency.find(packet.face) != it->second.transparency.cend());
                    packet.texture = face.texture;
                    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
                    enet_peer_send(peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
                }
            }

            protocol::packets::VoxelsChecksum packet;
            packet.checksum = globals::voxels.getChecksum();
            const std::vector<uint8_t> pbuf = protocol::serialize(packet);
            enet_peer_send(peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
        }
    },
    {
        protocol::packets::RequestChunks::id,
        [](const std::vector<uint8_t> &, SessionInfo *info, ENetPeer *peer) {
            const auto view = globals::registry.view<ChunkComponent>();
            for(const auto [entity, chunk] : view.each()) {
                if(ServerChunk *svc = globals::chunks.find(chunk.position)) {
                    protocol::packets::ChunkData packet;
                    packet.position = chunk.position;
                    packet.data = svc->data;
                    const std::vector<uint8_t> pbuf = protocol::serialize(packet);
                    enet_peer_send(peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));
                }
            }
        }
    },
    {
        protocol::packets::RequestSpawn::id,
        [](const std::vector<uint8_t> &, SessionInfo *info, ENetPeer *peer) {
            protocol::packets::ClientSpawn packet;
            packet.position = FLOAT3_ZERO;
            packet.head_angles = FLOAT3_ZERO;
            const std::vector<uint8_t> pbuf = protocol::serialize(packet);
            enet_peer_send(peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));

            // Create an entity
            info->entity = globals::registry.create();
            globals::registry.emplace<PlayerComponent>(info->entity);

            CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(info->entity);
            creature.position = packet.position;
            creature.orientation = FLOATQUAT_IDENTITY;

            HeadComponent &head = globals::registry.emplace<HeadComponent>(info->entity);
            head.angles = packet.head_angles;
            head.offset = FLOAT3_ZERO;
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

void sv_game::init()
{

}

void sv_game::postInit()
{

}

void sv_game::shutdown()
{

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
}
