/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460 core

layout(location = 0) in vec2 texcoord;
layout(location = 1) in float atlas_id;
layout(location = 2) in vec4 shadow_coord;

layout(location = 0) out vec4 color;

layout(binding = 0) uniform sampler2DArray atlas;
layout(binding = 1) uniform sampler2D shadowmap;

float calcShadow()
{
    const vec2 texel = 1.0 / vec2(2048.0, 2048.0);
    vec3 projcoord = (shadow_coord.xyz / shadow_coord.w) * 0.5 + 0.5;
    float shadow = 0.0;
    for(int i = -1; i < 2; i++) {
        for(int j = -1; j < 2; j++) {
            float pcf = texture(shadowmap, projcoord.xy + vec2(i, j) * texel).r;
            shadow += (projcoord.z - 0.0005) > pcf ? 0.0 : 1.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    color = texture(atlas, vec3(texcoord.xy, atlas_id)) * (0.5 + calcShadow());
}
