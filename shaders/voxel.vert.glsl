/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in uvec2 pack;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec3 texcoord;

layout(std140, binding = 0) uniform ubo {
    mat4 projview;
    vec3 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(chunkpos + (unpackUnorm4x8(pack.x).xyz * 16.0), 1.0);
    texcoord.xy = unpackUnorm4x8(pack.y).xy * 16.0;
    texcoord.z = float(pack.y >> 16);
}
