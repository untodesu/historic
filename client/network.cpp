/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <exception>
#include <client/chunks.hpp>
#include <client/globals.hpp>
#include <client/network.hpp>
#include <client/render/atlas.hpp>
#include <client/components/camera.hpp>
#include <client/components/local_player.hpp>
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
#include <shared/session.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>
#include <vector>
#include <unordered_map>

struct NetIDComponent final { uint32_t id; };
static std::unordered_map<uint32_t, entt::entity> network_entities;
static std::unordered_map<voxel_t, VoxelInfo> voxeldef_draft;

static void clearNetworkEntities()
{
    const auto view = globals::registry.view<NetIDComponent>();
    for(const auto [entity, netid] : view.each())
        globals::registry.destroy(entity);
    network_entities.clear();
}

static void draftVoxelEntry(voxel_t voxel, VoxelType type)
{
    if(voxel != NULL_VOXEL) {
        auto it = voxeldef_draft.find(voxel);
        VoxelInfo &info = (it != voxeldef_draft.end()) ? it->second : (voxeldef_draft[voxel] = VoxelInfo());
        info.type = type;
        info.transparency.clear();
        info.faces.clear();
    }
}

static void draftVoxelFace(voxel_t voxel, const VoxelFaceInfo &face, bool transparent)
{
    if(voxel != NULL_VOXEL) {
        auto it = voxeldef_draft.find(voxel);
        VoxelInfo &info = (it != voxeldef_draft.end()) ? it->second : (voxeldef_draft[voxel] = VoxelInfo());

        if(transparent)
            info.transparency.insert(face.face);
            
        for(VoxelFaceInfo &it : info.faces) {
            if(it.face == face.face) {
                it.texture = face.texture;
                return;
            }
        }

        info.faces.push_back(face);
    }
}

static void draftSubmit()
{
    globals::voxels.clear();
    for(const auto it : voxeldef_draft)
        globals::voxels.set(it.first, it.second);
}

static const std::unordered_map<uint16_t, void(*)(const std::vector<uint8_t> &)> packets = {
    {
        protocol::packets::GamedataChunkVoxels::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::GamedataChunkVoxels packet;
            protocol::deserialize(payload, packet);
            globals::chunks.create(math::arrayToVec<chunkpos_t>(packet.position), CHUNK_CREATE_UPDATE_NEIGHBOURS)->data = packet.data;
        }
    },
    {
        protocol::packets::GamedataEndRequest::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::GamedataEndRequest packet;
            protocol::deserialize(payload, packet);

            draftSubmit();
            voxeldef_draft.clear();

            uint64_t voxel_checksum = globals::voxels.getChecksum();
            if(packet.voxel_checksum != voxel_checksum) {
                // This is not THAT bad to request the
                // whole voxel description table again
                // so we stay only with a warning.
                spdlog::warn("VoxelDef checksums differ! (CL: {}, SV: {})", voxel_checksum, packet.voxel_checksum);
            }

            globals::solid_textures.create(32, 32, MAX_VOXELS);
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                for(const VoxelFaceInfo &face : it->second.faces)
                    globals::solid_textures.push(face.texture);
            }
            globals::solid_textures.submit();

            // UNDONE: do this in the client game loop!!!
            util::sendPacket(globals::session.peer, protocol::packets::RequestRespawn {}, 0, 0);
        }
    },
    {
        protocol::packets::GamedataVoxelEntry::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::GamedataVoxelEntry packet;
            protocol::deserialize(payload, packet);
            draftVoxelEntry(packet.voxel, packet.type);
        }
    },
    {
        protocol::packets::GamedataVoxelFace::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::GamedataVoxelFace packet;
            protocol::deserialize(payload, packet);

            VoxelFaceInfo info = {};
            info.face = packet.face;
            info.texture = packet.texture;

            draftVoxelFace(packet.voxel, info, !!packet.transparent);
        }
    },
    {
        protocol::packets::SpawnPlayer::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::SpawnPlayer packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = network::createEntity(packet.network_id);

            HeadComponent &head = globals::registry.emplace<HeadComponent>(entity);
            head.angles = math::arrayToVec<float3>(packet.head_angles);
            head.offset = FLOAT3_ZERO;

            CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(entity);
            creature.position = math::arrayToVec<float3>(packet.position);
            creature.yaw = packet.yaw;

            PlayerComponent &player = globals::registry.emplace<PlayerComponent>(entity);
            player.session_id = packet.session_id;

            // Local player?
            if(player.session_id == globals::session.id) {
                globals::registry.emplace<LocalPlayerComponent>(entity);
                globals::registry.emplace<ActiveCameraComponent>(entity);

                CameraComponent &camera = globals::registry.emplace<CameraComponent>(entity);
                camera.fov = glm::radians(90.0f);
                camera.z_near = 0.01f;
                camera.z_far = 1024.0f;

                globals::session.player_entity = entity;
                globals::session.player_network_id = packet.network_id;
                globals::session.state = SessionState::PLAYING;
            }
        }
    },
    {
        protocol::packets::LoginSuccess::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::LoginSuccess packet;
            protocol::deserialize(payload, packet);

            globals::session.state = SessionState::LOADING_GAMEDATA;
            globals::session.id = packet.session_id;

            spdlog::info("Logged in as {} with session_id={}", packet.username, globals::session.id);

            util::sendPacket(globals::session.peer, protocol::packets::RequestGamedata {}, 0, 0);
        }
    },
    {
        protocol::packets::CreaturePosition::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::CreaturePosition packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = network::findEntity(packet.network_id);
            if(globals::registry.valid(entity) && entity != globals::session.player_entity) {
                CreatureComponent &creature = globals::registry.get<CreatureComponent>(entity);
                creature.position = math::arrayToVec<float3>(packet.position);
            }
        }
    },
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);

            spdlog::info("Disconnected: {}", packet.reason);

            enet_peer_disconnect(globals::session.peer, 0);

            globals::session.peer = nullptr;
            globals::session.id = 0;
            globals::session.state = SessionState::DISCONNECTED;
            globals::session.player_entity = entt::null;
            globals::session.player_network_id = 0;

            globals::registry.clear();
        }
    },
    {
        protocol::packets::HeadAngles::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::HeadAngles packet;
            protocol::deserialize(payload, packet);

            entt::entity entity = network::findEntity(packet.network_id);
            if(globals::registry.valid(entity) && entity != globals::session.player_entity) {
                HeadComponent &head = globals::registry.get<HeadComponent>(entity);
                head.angles = math::arrayToVec<float3>(packet.angles);
            }
        }
    }
};

void cl_network::init()
{
    globals::host = enet_host_create(nullptr, 1, 2, 0, 0);
    if(!globals::host) {
        spdlog::error("Unable to create a client host object.");
        std::terminate();
    }

    globals::session.state = SessionState::DISCONNECTED;
}

void cl_network::shutdown()
{
    disconnect("Client shutting down");
    enet_host_destroy(globals::host);
    globals::host = nullptr;
}

bool cl_network::connect(const std::string &host, uint16_t port)
{
    disconnect("red vented");

    globals::registry.clear();

    ENetAddress address;
    address.port = port;
    if(enet_address_set_host(&address, host.c_str()) < 0) {
        spdlog::error("Unable to find {}:{}", host, port);
        return false;
    }

    globals::session.peer = enet_host_connect(globals::host, &address, 2, 0);
    if(!globals::session.peer) {
        spdlog::error("Unable to connect to {}:{}", host, port);
        return false;
    }

    ENetEvent event;
    while(enet_host_service(globals::host, &event, 5000) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::debug("Logging in...");

            protocol::packets::Handshake handshake = {};
            util::sendPacket(globals::session.peer, handshake, 0, 0);

            // TODO: change-able username
            protocol::packets::LoginStart login = {};
            login.username = "Kirill";
            util::sendPacket(globals::session.peer, login, 0, 0);

            globals::session.state = SessionState::LOGGING_IN;
            return true;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
            continue;
        }
    }

    enet_peer_reset(globals::session.peer);
    globals::session.peer = nullptr;

    spdlog::error("Unable to establish connection with {}:{}", host, port);
    return false;
}

void cl_network::disconnect(const std::string &reason)
{
    if(globals::session.peer) {
        protocol::packets::Disconnect packet;
        packet.reason = reason;
        util::sendPacket(globals::session.peer, packet, 0, 0);

        enet_peer_disconnect_later(globals::session.peer, 0);

        ENetEvent event;
        while(enet_host_service(globals::host, &event, 5000) > 0) {
            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected.");
                break;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                enet_packet_destroy(event.packet);
                continue;
            }
        }

        enet_peer_reset(globals::session.peer);
        globals::session.peer = nullptr;

        clearNetworkEntities();
    }
}

void cl_network::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            const std::vector<uint8_t> packet = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t packet_id;
            std::vector<uint8_t> payload;
            if(!protocol::split(packet, packet_id, payload)) {
                spdlog::warn("Invalid packet format!");
                continue;
            }

            const auto it = packets.find(packet_id);
            if(it == packets.cend()) {
                spdlog::warn("Invalid packet 0x{:04X}", packet_id);
                continue;
            }

            it->second(payload);
        }
    }
}

entt::entity cl_network::createEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend()) {
        spdlog::warn("Network entity {} alread exists!", network_id);
        return it->second;
    }

    entt::entity entity = globals::registry.create();
    globals::registry.emplace<NetIDComponent>(entity).id = network_id;
    return network_entities[network_id] = entity;
}

entt::entity cl_network::findEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend())
        return it->second;
    return entt::null;
}

void cl_network::removeEntity(uint32_t network_id)
{
    const auto it = network_entities.find(network_id);
    if(it != network_entities.cend()) {
        globals::registry.destroy(it->second);
        network_entities.erase(it);
    }
}
