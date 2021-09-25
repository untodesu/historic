/*
 * Copyright (c) 2021, Kirill GPRB. All Rights Reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#version 460

in VS_OUTPUT {
    vec2 texcoord;
} vert;

layout(location = 0) out vec4 target;

layout(binding = 0) uniform sampler2D albedo;
layout(binding = 1) uniform sampler2D normal;
layout(binding = 2) uniform sampler2D position;
layout(binding = 3) uniform sampler2D shadow_projcoord;
layout(binding = 4) uniform sampler2DShadow shadowmap;

layout(std140, binding = 0) uniform UBO_Composite {
    vec4 tweaks;
    vec4 light_direction;
    vec4 light_color;
    vec4 ambient;
};

// Algorithm shamelessly yoinked from Source SDK.
float doShadowNvidiaCheap(sampler2DShadow s, vec3 p, vec4 t)
{
    const vec2 epsilon = 1.0 / t.xy;
    vec4 taps = vec4(0.0, 0.0, 0.0, 0.0);
    taps.x = textureProj(s, vec4(p.xy + vec2( epsilon.x,  epsilon.y), p.z, 1.0));
    taps.y = textureProj(s, vec4(p.xy + vec2( epsilon.x, -epsilon.y), p.z, 1.0));
    taps.z = textureProj(s, vec4(p.xy + vec2(-epsilon.x,  epsilon.y), p.z, 1.0));
    taps.w = textureProj(s, vec4(p.xy + vec2(-epsilon.x, -epsilon.y), p.z, 1.0));
    return dot(taps, vec4(0.25));
}

void main()
{
    const float diffuse = max(dot(normalize(texture(normal, vert.texcoord).xyz), light_direction.xyz), 0.0);
    const float shadow = doShadowNvidiaCheap(shadowmap, texture(shadow_projcoord, vert.texcoord).xyz, tweaks);
    const vec3 lighting = mix(ambient.rgb, light_color.rgb, shadow) * diffuse;
    target = texture(albedo, vert.texcoord) * vec4(lighting, 1.0);
}
