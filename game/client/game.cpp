/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <common/math/const.hpp>
#include <common/math/math.hpp>
#include <exception>
#include <game/client/comp/camera.hpp>
#include <game/client/comp/local_player.hpp>
#include <game/client/sys/chunk_mesher.hpp>
#include <game/client/sys/chunk_renderer.hpp>
#include <game/client/sys/player_look.hpp>
#include <game/client/sys/player_move.hpp>
#include <game/client/sys/proj_view.hpp>
#include <game/client/util/screenshots.hpp>
#include <game/client/atlas.hpp>
#include <game/client/chunks.hpp>
#include <game/client/composite.hpp>
#include <game/client/debug_overlay.hpp>
#include <game/client/game.hpp>
#include <game/client/gbuffer.hpp>
#include <game/client/globals.hpp>
#include <game/client/input.hpp>
#include <game/client/screen.hpp>
#include <game/client/shadow_manager.hpp>
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
#include <spdlog/spdlog.h>
#include <unordered_map>

class VoxelDefBuilder final {
public:
    static inline void clear()
    {
        data.clear();
    }

    static inline void info(voxel_t voxel, VoxelType type)
    {
        if(voxel != NULL_VOXEL) {
            auto it = data.find(voxel);
            VoxelInfo &info = (it != data.end()) ? it->second : (data[voxel] = VoxelInfo());
            info.type = type;
            info.transparency.clear();
            info.faces.clear();
        }
    }

    static inline void face(voxel_t voxel, const VoxelFaceInfo &face, bool transparent)
    {
        if(voxel != NULL_VOXEL) {
            auto it = data.find(voxel);
            VoxelInfo &info = (it != data.end()) ? it->second : (data[voxel] = VoxelInfo());

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

    static inline void submit(VoxelDef &def)
    {
        def.clear();
        for(const auto it : data)
            def.set(it.first, it.second);
    }

private:
    static std::unordered_map<voxel_t, VoxelInfo> data;
};

std::unordered_map<voxel_t, VoxelInfo> VoxelDefBuilder::data;

using packet_handler_t = void(*)(const std::vector<uint8_t> &);
static const std::unordered_map<uint16_t, packet_handler_t> packet_handlers = {
    {
        protocol::packets::Disconnect::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::Disconnect packet;
            protocol::deserialize(payload, packet);

            spdlog::info("Disconnected: {}", packet.reason);

            enet_peer_disconnect(globals::peer, 0);

            globals::peer = nullptr;
            globals::session_id = 0;
            globals::state = ClientState::DISCONNECTED;
            globals::local_player = entt::null;

            globals::registry.clear();
        }
    },
    {
        protocol::packets::LoginSuccess::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::LoginSuccess packet;
            protocol::deserialize(payload, packet);

            globals::state = ClientState::RECEIVING_GAME_DATA;
            globals::session_id = packet.session_id;

            spdlog::info("Logged in with session_id={}", globals::session_id);

            VoxelDefBuilder::clear();

            util::sendPacket(globals::peer, protocol::packets::RequestVoxelDef {}, 0, ENET_PACKET_FLAG_RELIABLE);
            util::sendPacket(globals::peer, protocol::packets::RequestLoginChunks {}, 0, ENET_PACKET_FLAG_RELIABLE);
        }
    },
    {
        protocol::packets::VoxelDefVoxel::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefVoxel packet;
            protocol::deserialize(payload, packet);
            VoxelDefBuilder::info(packet.voxel, packet.type);
        }
    },
    {
        protocol::packets::VoxelDefFace::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefFace packet;
            protocol::deserialize(payload, packet);

            VoxelFaceInfo face = {};
            face.face = packet.face;
            face.texture = packet.texture;

            VoxelDefBuilder::face(packet.voxel, face, packet.transparent);
        }
    },
    {
        protocol::packets::VoxelDefEnd::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::VoxelDefEnd packet;
            protocol::deserialize(payload, packet);

            VoxelDefBuilder::submit(globals::voxels);
            VoxelDefBuilder::clear();

            const uint64_t checksum = globals::voxels.getChecksum();
            if(packet.checksum != checksum) {
                // This is not THAT bad to request the
                // whole voxel description table again
                // so we stay only with a warning.
                spdlog::warn("VoxelDef checksums differ! (CL: {}, SV: {})", checksum, packet.checksum);
            }

            globals::solid_textures.create(32, 32, MAX_VOXELS);
            for(VoxelDef::const_iterator it = globals::voxels.cbegin(); it != globals::voxels.cend(); it++) {
                for(const VoxelFaceInfo &face : it->second.faces)
                    globals::solid_textures.push(face.texture);
            }
            globals::solid_textures.submit();
        }
    },
    {
        protocol::packets::ChunkData::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::ChunkData packet;
            protocol::deserialize(payload, packet);
            globals::chunks.create(math::arrayToVec<chunkpos_t>(packet.position), CHUNK_CREATE_UPDATE_NEIGHBOURS)->data = packet.data;
        }
    },
    {
        protocol::packets::LoginChunksEnd::id,
        [](const std::vector<uint8_t> &) {
            // TODO: request entity data here.
            util::sendPacket(globals::peer, protocol::packets::RequestSpawn {}, 0, ENET_PACKET_FLAG_RELIABLE);
        }
    },
    {
        protocol::packets::SpawnPlayer::id,
        [](const std::vector<uint8_t> &payload) {
            protocol::packets::SpawnPlayer packet;
            protocol::deserialize(payload, packet);

            entt::entity player = globals::registry.create();
            globals::registry.emplace<PlayerComponent>(player);
            globals::registry.emplace<LocalPlayerComponent>(player);

            CreatureComponent &creature = globals::registry.emplace<CreatureComponent>(player);
            creature.orientation = FLOATQUAT_IDENTITY;
            creature.position = math::arrayToVec<float3>(packet.position);

            HeadComponent &head = globals::registry.emplace<HeadComponent>(player);
            head.angles = math::arrayToVec<float2>(packet.head_angles);
            head.offset = FLOAT3_ZERO;

            if(packet.session_id == globals::session_id && globals::state != ClientState::PLAYING) {
                globals::registry.emplace<ActiveCameraComponent>(player);
                CameraComponent &camera = globals::registry.emplace<CameraComponent>(player);
                camera.fov = glm::radians(90.0f);
                camera.z_near = 0.01f;
                camera.z_far = 1024.0f;
                globals::state = ClientState::PLAYING;
                globals::local_player = player;
            }
        }
    }
};

void cl_game::init()
{
    chunk_renderer::init();
    composite::init();

    shadow_manager::init(8192, 8192);
    shadow_manager::setLightOrientation(floatquat(glm::radians(float3(45.0f, 0.0f, 45.0f))));
    shadow_manager::setPolygonOffset(float2(3.0f, 0.5f));
}

void cl_game::postInit()
{
    cl_game::connect("localhost", protocol::DEFAULT_PORT);
}

void cl_game::shutdown()
{
    cl_game::disconnect("cl_game::shutdown");

    globals::registry.clear();

    globals::solid_textures.destroy();

    globals::solid_gbuffer.shutdown();

    shadow_manager::shutdown();

    composite::shutdown();
    chunk_renderer::shutdown();
}

void cl_game::modeChange(int width, int height)
{
    globals::solid_gbuffer.init(width, height);
}

bool cl_game::connect(const std::string &host, uint16_t port)
{
    cl_game::disconnect("cl_game::connect");

    globals::registry.clear();

    ENetAddress address;
    address.port = port;
    if(enet_address_set_host(&address, host.c_str()) < 0) {
        spdlog::error("Unable to find {}:{}", host, port);
        return false;
    }

    globals::peer = enet_host_connect(globals::host, &address, 2, 0);
    if(!globals::peer) {
        spdlog::error("Unable to connect to {}:{}", host, port);
        return false;
    }

    ENetEvent event;
    while(enet_host_service(globals::host, &event, 5000) > 0) {
        if(event.type == ENET_EVENT_TYPE_CONNECT) {
            spdlog::debug("Logging in...");

            // Send Handshake
            protocol::packets::Handshake handshake;
            const std::vector<uint8_t> hpbuf = protocol::serialize(handshake);
            enet_peer_send(globals::peer, 0, enet_packet_create(hpbuf.data(), hpbuf.size(), ENET_PACKET_FLAG_RELIABLE));
            
            // Send LoginStart
            // TODO: change-able username
            protocol::packets::LoginStart login;
            login.username = "Kirill";
            const std::vector<uint8_t> lpbuf = protocol::serialize(login);
            enet_peer_send(globals::peer, 0, enet_packet_create(lpbuf.data(), lpbuf.size(), ENET_PACKET_FLAG_RELIABLE));

            globals::state = ClientState::LOGGING_IN;
            return true;
        }

        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            enet_packet_destroy(event.packet);
            continue;
        }
    }

    enet_peer_reset(globals::peer);
    globals::peer = nullptr;

    spdlog::error("Unable to establish connection with {}:{}", host, port);
    return false;
}

void cl_game::disconnect(const std::string &reason)
{
    if(globals::peer) {
        // Send Disconnect
        protocol::packets::Disconnect packet;
        packet.reason = reason;
        const std::vector<uint8_t> pbuf = protocol::serialize(packet);
        enet_peer_send(globals::peer, 0, enet_packet_create(pbuf.data(), pbuf.size(), ENET_PACKET_FLAG_RELIABLE));

        enet_peer_disconnect_later(globals::peer, 0);

        ENetEvent event;
        while(enet_host_service(globals::host, &event, 5000) > 0) {
            if(event.type == ENET_EVENT_TYPE_DISCONNECT) {
                spdlog::info("Disconnected.");
                break;
            }

            if(event.type == ENET_EVENT_TYPE_RECEIVE) {
                spdlog::debug("Unwanted (right now) packet with the size of {:.02f} KiB", static_cast<float>(event.packet->dataLength) / 1024.0f);
                enet_packet_destroy(event.packet);
                continue;
            }
        }

        enet_peer_reset(globals::peer);
        globals::peer = nullptr;
    }
}

void cl_game::update()
{
    ENetEvent event;
    while(enet_host_service(globals::host, &event, 0) > 0) {
        if(event.type == ENET_EVENT_TYPE_RECEIVE) {
            const std::vector<uint8_t> packet = std::vector<uint8_t>(event.packet->data, event.packet->data + event.packet->dataLength);
            enet_packet_destroy(event.packet);

            uint16_t type;
            std::vector<uint8_t> payload;
            if(!protocol::split(packet, type, payload)) {
                spdlog::warn("Invalid packet format!");
                continue;
            }

            const auto it = packet_handlers.find(type);
            if(it == packet_handlers.cend()) {
                spdlog::warn("Invalid packet 0x{:04X}", type);
                continue;
            }

            it->second(payload);
        }
    }

    proj_view::update();

    // NOTENOTE: when the new chunks arrive (during the login stage
    // when clientside receives some important data like voxels)
    // sometimes shit gets fucked and one side of a chunk becomes
    // visible. This is not a problem for now because these quads
    // are occluded by the actual geometry. Things will get hot only
    // when mesher will start to skip actual geometry leaving holes.
    chunk_mesher::update();

    bool is_playing = (globals::state == ClientState::PLAYING);
    if(is_playing) {
        player_look::update();
        player_move::update();
    }

    input::enableCursor(!is_playing);
}

void cl_game::draw()
{
    // Draw things to GBuffers
    chunk_renderer::draw();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    int width, height;
    screen::getSize(width, height);
    glViewport(0, 0, width, height);

    // Draw things to the main framebuffer
    composite::draw();
}

void cl_game::drawImgui()
{
    debug_overlay::draw();
}

void cl_game::postDraw()
{
    if(input::isKeyJustPressed(GLFW_KEY_F2))
        screenshots::jpeg(100);
}
