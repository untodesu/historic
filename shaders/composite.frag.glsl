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
layout(binding = 3) uniform sampler2D shadow_projcoord;
layout(binding = 4) uniform sampler2D shadowmap;

layout(std140, binding = 0) uniform UBO_Composite {
    vec4 camera;
    vec4 light_pos;
    vec4 ambient;
};

float calcShadow(vec3 projcoord)
{
    const vec2 texel = 1.0 / vec2(4096.0, 4096.0);
    float shadow = 0.0;
    for(int i = -1; i < 2; i++) {
        for(int j = -1; j < 2; j++) {
            float pcf = texture(shadowmap, projcoord.xy + vec2(i, j) * texel).r;
            shadow += projcoord.z + length(texel) > pcf ? 0.0 : 1.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec4 result = texture(albedo, vso.texcoord);
    float lighting = 0.0;

    vec3 norm = normalize(texture(normal, vso.texcoord).rgb);
    vec3 light_dir = normalize(light_pos.xyz - texture(position, vso.texcoord).rgb);
    lighting += max(dot(norm, light_dir), 0.0);

    lighting += ambient.x;

    lighting += calcShadow(texture(shadow_projcoord, vso.texcoord).rgb);

    result.rgb *= lighting;
    target = result;
}
