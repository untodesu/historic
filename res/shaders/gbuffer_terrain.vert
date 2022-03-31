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
    float shade;
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
    vec3 position = vec3(0.0, 0.0, 0.0);
    position.x = float((pack_0 >> 27) & 0x1F);
    position.y = float((pack_0 >> 22) & 0x1F);
    position.z = float((pack_0 >> 17) & 0x1F);
    position += cpos_world.xyz;
    vec2 texcoord = unpackUnorm4x8(pack_0 & 0xFFFF).xy;
    vec3 normal = vec3(0.0, 0.0, 0.0);
    normal.z = float((pack_1 >> 20) & 0x3FF) / 1023.0 * 2.0 - 1.0;
    normal.y = float((pack_1 >> 10) & 0x3FF) / 1023.0 * 2.0 - 1.0;
    normal.x = float(pack_1 & 0x3FF) / 1023.0 * 2.0 - 1.0;
    vert.texcoord = vec3(texcoord, floor(float(pack_2) + 0.5));
    const float shades[4] = { 0.4, 0.6, 0.8, 1.0 };
    vert.shade = shades[(pack_1 >> 30) & 0x03];
    vert.normal = normalize(normal);
    gl_Position = vpmatrix * vec4(position, 1.0);
}
