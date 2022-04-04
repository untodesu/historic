/*
 * Copyright (c) 2022 Kirill GPRB
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460 core

out VERT_OUTPUT {
    vec2 texcoord;
} vert;

out gl_PerVertex {
    vec4 gl_Position;
};

const vec2 positions[4] = vec2[4](
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0),
    vec2( 1.0, -1.0)
);

const vec2 texcoords[4] = vec2[4](
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    vec2(1.0, 0.0)
);

void main()
{
    gl_Position = vec4(positions[gl_VertexID % 4], 0.0, 1.0);
    vert.texcoord = texcoords[gl_VertexID % 4];
}
