/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;

out gl_PerVertex { vec4 gl_Position; };

layout(std140, binding = 0) uniform ubo {
    mat4 projview;
    mat4 projview_shadow;
    vec3 chunkpos;
};

void main()
{
    gl_Position = projview_shadow * vec4(chunkpos + position, 1.0);
}
