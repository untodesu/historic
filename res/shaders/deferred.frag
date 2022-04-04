/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460 core

in VERT_OUTPUT {
    vec2 texcoord;
} vert;

layout(location = 0) out vec4 target;

layout(binding = 0) uniform sampler2D albedo;
layout(binding = 1) uniform sampler2D normal;

void main()
{
    // We still lack voxel lighting so
    // here are some constants so we don't
    // just render shit in fullbright.
    const vec3 AMBIENT_COLOR = vec3(0.2, 0.2, 0.2);
    const vec3 CONST_LIGHT_DIR = normalize(vec3(-0.5, 1.0, -0.25));

    const vec4 v_albedo = texture(albedo, vert.texcoord);
    const vec3 v_normal = normalize(texture(normal, vert.texcoord).rgb);
    const float diffuse_1 = max(dot(v_normal, CONST_LIGHT_DIR), 0.0);

    const vec3 lighting = diffuse_1 * v_albedo.rgb + v_albedo.rgb * AMBIENT_COLOR;

    target = vec4(lighting, v_albedo.a);
}
