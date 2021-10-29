/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <client/atlas.hpp>
#include <client/chunks.hpp>
#include <client/globals.hpp>
#include <client/net_gamedata.hpp>
#include <shared/protocol/packets/client/request_respawn.hpp>
#include <shared/protocol/packets/server/gamedata_chunk_voxels.hpp>
#include <shared/protocol/packets/server/gamedata_end_request.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_entry.hpp>
#include <shared/protocol/packets/server/gamedata_voxel_face.hpp>
#include <shared/util/enet.hpp>
#include <shared/voxels.hpp>
#include <spdlog/spdlog.h>

namespace voxeldef_builder
{
static std::unordered_map<voxel_t, VoxelInfo> data;

static inline void entry(voxel_t voxel, VoxelType type)
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

static inline void submit()
{
    globals::voxels.clear();
    for(const auto it : data)
        globals::voxels.set(it.first, it.second);
}
} // namespace voxeldef_builder

static void onChunkVoxels(const std::vector<uint8_t> &payload)
{
    protocol::packets::GamedataChunkVoxels packet;
    protocol::deserialize(payload, packet);
    globals::chunks.create(math::arrayToVec<chunkpos_t>(packet.position), CHUNK_CREATE_UPDATE_NEIGHBOURS)->data = packet.data;
}

static void onEndRequest(const std::vector<uint8_t> &payload)
{
    protocol::packets::GamedataEndRequest packet;
    protocol::deserialize(payload, packet);

    voxeldef_builder::submit();
    voxeldef_builder::data.clear();

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
    util::sendPacket(globals::peer, protocol::packets::RequestRespawn {}, 0, 0);
}

static void onVoxelEntry(const std::vector<uint8_t> &payload)
{
    protocol::packets::GamedataVoxelEntry packet;
    protocol::deserialize(payload, packet);
    voxeldef_builder::entry(packet.voxel, packet.type);
}

static void onVoxelFace(const std::vector<uint8_t> &payload)
{
    protocol::packets::GamedataVoxelFace packet;
    protocol::deserialize(payload, packet);

    VoxelFaceInfo info = {};
    info.face = packet.face;
    info.texture = packet.texture;

    voxeldef_builder::face(packet.voxel, info, !!packet.transparent);
}

void net_gamedata::init()
{
    globals::packet_handlers[protocol::packets::GamedataChunkVoxels::id] = &onChunkVoxels;
    globals::packet_handlers[protocol::packets::GamedataEndRequest::id] = &onEndRequest;
    globals::packet_handlers[protocol::packets::GamedataVoxelEntry::id] = &onVoxelEntry;
    globals::packet_handlers[protocol::packets::GamedataVoxelFace::id] = &onVoxelFace;
    voxeldef_builder::data.clear();
}
