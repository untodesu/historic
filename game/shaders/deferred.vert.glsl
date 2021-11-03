/*
 * deferred.vert.glsl
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

out VS_OUTPUT {
    vec2 texcoord;
} vert;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    vert.texcoord = texcoord;
    gl_Position = vec4(position, 0.0, 1.0);
}
