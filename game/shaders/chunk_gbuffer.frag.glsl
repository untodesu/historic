/*
 * chunk_gbuffer.frag.glsl
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#version 460

in VS_OUTPUT {
    vec3 texcoord;
    vec3 normal;
    vec3 position;
    vec4 shadow_projcoord;
    float side_shade;
} vert;

layout(location = 0) out vec4 albedo;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 position;
layout(location = 3) out vec3 shadow_projcoord;

layout(binding = 0) uniform sampler2DArray atlas;

void main()
{
    albedo = texture(atlas, vert.texcoord);
    normal = normalize(vert.normal);
    position = vert.position;
    shadow_projcoord = vert.shadow_projcoord.xyz;
}
