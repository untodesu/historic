/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in uint pack_0;
layout(location = 1) in uint pack_1;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec2 texcoord;
layout(location = 1) out float atlas_id;

layout(std140, binding = 0) uniform ubo {
    mat4 projview;
    vec3 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(chunkpos + (unpackUnorm4x8(pack_0).xyz * 16.0), 1.0);
    texcoord = unpackUnorm4x8(pack_1).xy * 16.0;
    atlas_id = max(0.0, floor(float(pack_1 >> 16) + 0.5));
}
