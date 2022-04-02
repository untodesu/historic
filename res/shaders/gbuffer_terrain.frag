/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460 core

in VERT_OUTPUT {
    float shade;
    vec3 texcoord;
    vec3 normal;
} vert;

layout(location = 0) out vec4 gbuffer_albedo;
layout(location = 1) out vec3 gbuffer_normal;

layout(binding = 0) uniform sampler2DArray atlas;

void main(void)
{
    gbuffer_albedo = texture(atlas, vert.texcoord);
    gbuffer_albedo.rgb *= vert.shade;
    gbuffer_normal = normalize(vert.normal);
}
