/*
 * chunk_shadow.vert.glsl
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#version 460

layout(location = 0) in vec3 position;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(std140, binding = 0) uniform __ubo_0 {
    mat4 projview;
    vec4 chunkpos;
};

void main()
{
    gl_Position = projview * vec4(chunkpos.xyz + position, 1.0);
}
