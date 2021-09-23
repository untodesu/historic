/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texcoord;

out VS_OUTPUT {
    vec2 texcoord;
} vso;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    vso.texcoord = texcoord;
    gl_Position = vec4(position, 0.0, 1.0);
}
