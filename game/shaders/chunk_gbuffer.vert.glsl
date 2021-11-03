/*
 * chunk_gbuffer.vert.glsl
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;
layout(location = 3) in uint atlas_id;

out VS_OUTPUT {
    vec3 texcoord;
    vec3 normal;
    vec3 position;
    vec4 shadow_projcoord;
} vert;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(std140, binding = 0) uniform __ubo_0 {
    mat4 projview;
    mat4 projview_shadow;
    vec4 chunkpos;
};

void main()
{
    vert.texcoord = vec3(texcoord, max(0.0, floor(float(atlas_id) + 0.5)));

    vert.normal = normalize(normal);

    vert.position = chunkpos.xyz + position;

    vert.shadow_projcoord = projview_shadow * vec4(chunkpos.xyz + position, 1.0);
    vert.shadow_projcoord.xyz /= vert.shadow_projcoord.w;
    vert.shadow_projcoord.xyz *= 0.5;
    vert.shadow_projcoord.xyz += 0.5;

    gl_Position = projview * vec4(vert.position, 1.0);
}
