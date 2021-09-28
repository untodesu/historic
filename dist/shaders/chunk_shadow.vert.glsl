/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in vec3 position;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(std140, binding = 0) uniform UBO_Shadow {
    mat4 projview;
    vec4 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(chunkpos.xyz + position, 1.0);
}
