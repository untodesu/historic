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
layout(binding = 4) uniform sampler2DShadow shadowmap;

layout(std140, binding = 0) uniform UBO_Composite {
    vec4 camera_position;
    vec4 light_direction;
    vec4 light_color;
    vec4 ambient_color;
};

float doShadow(sampler2DShadow s, vec3 p)
{
    // This should not be hardcoded
    const float epsilon = 1.0 / 8192.0;

    vec4 taps;
    taps.x = textureProj(s, vec4(p.xy + vec2( epsilon,  epsilon), p.z, 1.0));
    taps.y = textureProj(s, vec4(p.xy + vec2( epsilon, -epsilon), p.z, 1.0));
    taps.z = textureProj(s, vec4(p.xy + vec2(-epsilon,  epsilon), p.z, 1.0));
    taps.w = textureProj(s, vec4(p.xy + vec2(-epsilon, -epsilon), p.z, 1.0));
    return dot(taps, vec4(0.25));
}

void main()
{
    vec4 color = texture(albedo, vso.texcoord);
    vec3 lighting = vec3(0.0, 0.0, 0.0);
    vec3 norm = normalize(texture(normal, vso.texcoord).xyz);
    vec3 projcoord = texture(shadow_projcoord, vso.texcoord).xyz;

    lighting += ambient_color.rgb;
    //lighting += calcShadow(projcoord) * max(dot(norm, light_direction.xyz), 0.0);
    //lighting += textureProj(shadowmap, projcoord) * light_color.xyz * max(dot(norm, light_direction.xyz), 0.0);
    lighting += doShadow(shadowmap, projcoord) * max(dot(norm, light_direction.xyz), 0.0);

    color.xyz *= lighting;
    target = color;
}
