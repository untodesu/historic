/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace protocol
{
enum class SessionStatus {
    // CL:  A session is not connected
    //      to any servers out there.
    // SV:  No behaviour.
    DISCONNECT,

    // CL:  A session has sent a Handshake packet
    //      and a LoginInfo packet; awaits for
    //      the server's response for that.
    // SV:  A session has sent a Handshake packet;
    //      awaits for a LoginInfo packet.
    CONNECTING,

    // CL:  Awaits for either a LoginSuccess packet
    //      and sends the LoginSuccessAck packet, or
    //      awaits for a Disconnect packet.
    // SV:  A session has sent a LoginInfo packet;
    //      either sends a LoginSuccess packet and
    //      awaits for LoginSuccessAck packet or
    //      sends a Disconnected packet.
    LOGGING_IN,

    // CL:  Awaits for core server information packets
    //      like VoxelDef family; after that awaits for
    //      world data packets (voxel data, entity packets);
    //      This stage ends with receiving a FinishDownloading packet.
    // SV:  Sends core server information packets; starting with
    //      the next tick, sends world data packets with a
    //      fixed rate of packets per tick; sends a FinishedDownloading
    //      packet once everything is transmitted, ending this stage.
    DOWNLOADING,

    // CL:  Receives client input, updates positions, transmits
    //      movement and event packets to the server. Awaits for
    //      server movement, event and world packets.
    // SV:  Awaits for movement and event packets from the client;
    //      broadcasts movement, event and world packets.
    PLAYING
};

enum class DownloadStage {
    // Voxel definition table, game rules,
    // general world and server information.
    SERVER_DATA,

    // Loaded chunks around the client player.
    // Possibly ends with a checksum packet.
    VOXEL_DATA,

    // Entity packets describing entities that
    // get spawned/removed/updated in the loaded chunks.
    ENTITIES,
};
} // namespace protocol
