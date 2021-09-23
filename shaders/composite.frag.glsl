/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

in VS_OUTPUT {
    vec2 texcoord;
} vso;

layout(location = 0) out vec4 target;

layout(binding = 0) uniform sampler2D albedo;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D position;
layout(binding = 3) uniform sampler2D shadowmap;

layout(std140, binding = 0) uniform UBO_Composite {
    mat4 projview_shadow;
};

float calcShadow(vec4 shadow_coord)
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
    vec4 shadow_coord = projview_shadow * texture(position, vso.texcoord);
    vec4 result = texture(albedo, vso.texcoord);
    result.xyz *= calcShadow(shadow_coord) + 0.25;
    target = result;
}
