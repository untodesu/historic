/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

layout(location = 0) in uvec2 pack;
layout(location = 0) out vec3 out_uv;

#if defined(_UVRE_)
out gl_PerVertex { vec4 gl_Position; };
#endif

layout(std140, binding = 0) uniform UBO {
    mat4x4 projview;
    vec3 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(unpackUnorm4x8(pack.x & 0x00FFFFFF).xyz * 64.0 + chunkpos, 1.0);
    out_uv.xy = unpackUnorm4x8(pack.y & 0x0000FFFF).xy;
    out_uv.z = max(0.0, floor(float((pack.y >> 16) & 0xFFFF) + 0.5));
}
