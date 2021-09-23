/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in uint atlas_id;

out VS_OUTPUT {
    vec3 model;
    vec3 normal;
    vec3 texcoord;
} vso;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(std140, binding = 0) uniform UBO_GBuffer {
    mat4 projview;
    mat4 projview_shadow;
    vec3 chunkpos;
};

void main()
{
    vso.model = chunkpos + position;
    vso.normal = normalize(normal);
    vso.texcoord = vec3(texcoord, max(0.0, floor(float(atlas_id) + 0.5)));
    gl_Position = projview * vec4(vso.model, 1.0);
}
