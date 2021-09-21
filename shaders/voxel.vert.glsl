/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in uvec3 pack;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec3 model;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 texcoord;
layout(location = 3) out float atlas_id;

layout(std140, binding = 0) uniform ubo {
    mat4 projview;
    vec3 chunkpos;
};

void main()
{
    model = chunkpos + (unpackUnorm4x8(pack.x).xyz * 16.0);
    normal = unpackSnorm4x8(pack.y).xyz;
    texcoord = unpackUnorm4x8(pack.z).xy * 16.0;
    atlas_id = max(0.0, floor(float(pack.z >> 16) + 0.5));
    gl_Position = projview * vec4(model, 1.0);
}
