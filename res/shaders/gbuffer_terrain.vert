/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460 core

layout(location = 0) in uint pack_0;
layout(location = 1) in uint pack_1;
layout(location = 2) in uint pack_2;

out VERT_OUTPUT {
    vec3 texcoord;
    vec3 normal;
} vert;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(std140, binding = 0) uniform gbuffer_uniforms {
    mat4 vpmatrix;
    vec4 cpos_world;
};

void main(void)
{
    gl_Position = vec4(cpos_world.xyz, 1.0);
    gl_Position.x += float((pack_0 >> 22) & 0x3FF) / 512.0 * 16.0;
    gl_Position.y += float((pack_0 >> 12) & 0x3FF) / 512.0 * 16.0;
    gl_Position.z += float((pack_0 >> 2) & 0x3FF) / 512.0 * 16.0;
    gl_Position = vpmatrix * gl_Position;

    vert.texcoord.x = float((pack_1 >> 27) & 0x1F) / 16.0;
    vert.texcoord.y = float((pack_1 >> 22) & 0x1F) / 16.0;
    vert.texcoord.z = floor(float(pack_2) + 0.5);

    vert.normal.x = float((pack_1 >> 14) & 0x7F) / 127.0 * 2.0 - 1.0;
    vert.normal.y = float((pack_1 >> 7) & 0x7F) / 127.0 * 2.0 - 1.0;
    vert.normal.z = float(pack_1 & 0x7F) / 127.0 * 2.0 - 1.0;
}
