/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in uint altas_id;

out gl_PerVertex { vec4 gl_Position; };

layout(location = 0) out vec2 frag_texcoord;
layout(location = 1) out float frag_atlas_id;

layout(std140, binding = 0) uniform ubo {
    mat4 projview;
    vec3 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(chunkpos + position, 1.0);
    frag_texcoord = texcoord;
    frag_atlas_id = max(0.0, floor(float(altas_id) + 0.5));
}
