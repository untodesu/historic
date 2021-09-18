/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

layout(location = 0) in uvec2 pos_uv;
layout(location = 1) in uint tex_id;

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
    gl_Position.xy = unpackUnorm2x16(pos_uv.x);
    gl_Position.zw = unpackUnorm2x16(pos_uv.y);
    gl_Position = projview * vec4((gl_Position.xyz * 64.0) + chunkpos, 1.0);
    fs_texcoord.xy = unpackUnorm4x8(pos_uv.y).zw;
    fs_texcoord.z = max(0.0, floor(float(tex_id) + 0.5));
}
