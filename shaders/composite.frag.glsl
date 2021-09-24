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
    vec4 camera_position;
    vec4 light_direction;
    vec4 light_color;
    vec4 ambient_color;
};

vec3 calcShadow(vec3 projcoord, float diffuse, float bias)
{
    const vec2 poisson[4] = {
        vec2(-0.94201624, -0.39906216),
        vec2( 0.94558609, -0.76890725),
        vec2(-0.09418410, -0.92938870),
        vec2( 0.34495938,  0.29387760),
    };

    vec3 result = vec3(1.0);
    for(int i = 0; i < 4; i++) {
        if(texture(shadowmap, projcoord.xy + poisson[i] / 700.0).r >= projcoord.z - 0.00006)
            continue;
        result -= 0.2;
    }

    return diffuse * result;
}

void main()
{
    vec4 color = texture(albedo, vso.texcoord);
    vec3 lighting = vec3(0.0, 0.0, 0.0);
    vec3 norm = normalize(texture(normal, vso.texcoord).rgb);
    vec3 projcoord = texture(shadow_projcoord, vso.texcoord).rgb;

    lighting += ambient_color.rgb;
    lighting += calcShadow(projcoord, max(dot(norm, light_direction.xyz), 0.0), 0);

    color.xyz *= lighting;
    target = color;
}
