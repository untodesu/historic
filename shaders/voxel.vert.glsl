/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in uint atlas_id;

layout(location = 0) out vec3 fs_texcoord;

#if defined(_UVRE_)
out gl_PerVertex { vec4 gl_Position; };
#endif

layout(std140, binding = 0) uniform UBO {
    mat4x4 projview;
    vec3 chunkpos;
};

void main()
{
    fs_texcoord.xy = texcoord;
    fs_texcoord.z = max(0.0, floor(float(atlas_id) + 0.5));
    gl_Position = projview * vec4(chunkpos + position, 1.0);
}
