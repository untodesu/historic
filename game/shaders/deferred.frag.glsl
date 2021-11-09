/*
 * deferred.frag.glsl
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
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

layout(std140, binding = 0) uniform __ubo_0 {
    vec4 tweaks;
    vec4 light_direction;
    vec4 light_color;
    vec4 ambient;
};

// Algorithm shamelessly yoinked from Source SDK.
float doShadowNvidiaCheap(sampler2DShadow s, vec3 p, vec4 t)
{
    const vec2 epsilon = 0.25 / t.xy;
    vec4 taps = vec4(0.0, 0.0, 0.0, 0.0);
    taps.x = textureProj(s, vec4(p.xy + vec2( epsilon.x,  epsilon.y), p.z, 1.0));
    taps.y = textureProj(s, vec4(p.xy + vec2( epsilon.x, -epsilon.y), p.z, 1.0));
    taps.z = textureProj(s, vec4(p.xy + vec2(-epsilon.x,  epsilon.y), p.z, 1.0));
    taps.w = textureProj(s, vec4(p.xy + vec2(-epsilon.x, -epsilon.y), p.z, 1.0));
    return dot(taps, vec4(0.25));
}

float doShadowNvidiaPCF5x5Gaussian(sampler2DShadow s, vec3 p, vec4 t)
{
    const vec2 epsilon_1 = 1.0 / t.xy;
    const vec2 epsilon_2 = 2.0 / t.xy;

    vec4 one_taps;
    one_taps.x = textureProj(s, vec4(p.xy + vec2( epsilon_2.x,  epsilon_2.y), p.z, 1.0));
    one_taps.y = textureProj(s, vec4(p.xy + vec2(-epsilon_2.x,  epsilon_2.y), p.z, 1.0));
    one_taps.z = textureProj(s, vec4(p.xy + vec2( epsilon_2.x, -epsilon_2.y), p.z, 1.0));
    one_taps.w = textureProj(s, vec4(p.xy + vec2(-epsilon_2.x, -epsilon_2.y), p.z, 1.0));
    float one_taps_f = dot(one_taps, vec4(1.0 / 331.0));

    vec4 seven_taps;
    seven_taps.x = textureProj(s, vec4(p.xy + vec2( epsilon_2.x, 0.0), p.z, 1.0));
    seven_taps.y = textureProj(s, vec4(p.xy + vec2(-epsilon_2.x, 0.0), p.z, 1.0));
    seven_taps.z = textureProj(s, vec4(p.xy + vec2(0.0, -epsilon_2.y), p.z, 1.0));
    seven_taps.w = textureProj(s, vec4(p.xy + vec2(0.0, -epsilon_2.y), p.z, 1.0));
    float seven_taps_f = dot(seven_taps, vec4(7.0 / 331.0));

    vec4 four_taps_a, four_taps_b;
    four_taps_a.x = textureProj(s, vec4(p.xy + vec2( epsilon_2.x,  epsilon_1.y), p.z, 1.0));
    four_taps_a.y = textureProj(s, vec4(p.xy + vec2( epsilon_1.x,  epsilon_2.y), p.z, 1.0));
    four_taps_a.z = textureProj(s, vec4(p.xy + vec2(-epsilon_1.x,  epsilon_2.y), p.z, 1.0));
    four_taps_a.w = textureProj(s, vec4(p.xy + vec2(-epsilon_2.x,  epsilon_1.y), p.z, 1.0));
    four_taps_b.x = textureProj(s, vec4(p.xy + vec2(-epsilon_2.x, -epsilon_1.y), p.z, 1.0));
    four_taps_b.y = textureProj(s, vec4(p.xy + vec2(-epsilon_1.x, -epsilon_2.y), p.z, 1.0));
    four_taps_b.z = textureProj(s, vec4(p.xy + vec2( epsilon_1.x, -epsilon_2.y), p.z, 1.0));
    four_taps_b.w = textureProj(s, vec4(p.xy + vec2( epsilon_2.x, -epsilon_1.y), p.z, 1.0));
    float four_taps_a_f = dot(four_taps_a, vec4(4.0 / 331.0));
    float four_taps_b_f = dot(four_taps_b, vec4(4.0 / 331.0));

    vec4 n_20_taps;
    n_20_taps.x = textureProj(s, vec4(p.xy + vec2( epsilon_1.x,  epsilon_1.y), p.z, 1.0));
    n_20_taps.y = textureProj(s, vec4(p.xy + vec2(-epsilon_1.x,  epsilon_1.y), p.z, 1.0));
    n_20_taps.z = textureProj(s, vec4(p.xy + vec2( epsilon_1.x, -epsilon_1.y), p.z, 1.0));
    n_20_taps.w = textureProj(s, vec4(p.xy + vec2(-epsilon_1.x, -epsilon_1.y), p.z, 1.0));
    float n_20_taps_f = dot(n_20_taps, vec4(20.0 / 331.0));

    vec4 n_33_taps;
    n_33_taps.x = textureProj(s, vec4(p.xy + vec2( epsilon_1.x, 0.0), p.z, 1.0));
    n_33_taps.y = textureProj(s, vec4(p.xy + vec2(-epsilon_1.x, 0.0), p.z, 1.0));
    n_33_taps.z = textureProj(s, vec4(p.xy + vec2(0.0, -epsilon_1.y), p.z, 1.0));
    n_33_taps.w = textureProj(s, vec4(p.xy + vec2(0.0, -epsilon_1.y), p.z, 1.0));
    float n_33_taps_f = dot(n_33_taps, vec4(33.0 / 331.0));

    float center_tap = textureProj(s, vec4(p, 1.0)) * (55.0 / 331.0);

    return one_taps_f + seven_taps_f + four_taps_a_f + four_taps_b_f + n_20_taps_f + n_33_taps_f + center_tap;
}

void main()
{
    const vec3 normal_v = normalize(texture(normal, vert.texcoord).xyz);
    const float diffuse = max(dot(normal_v, normalize(light_direction.xyz)), 0.0);
    const float shadow = doShadowNvidiaPCF5x5Gaussian(shadowmap, texture(shadow_projcoord, vert.texcoord).xyz, tweaks);
    const vec3 lighting = ambient.rgb + light_color.rgb * shadow * diffuse;
    target = texture(albedo, vert.texcoord) * vec4(lighting, 1.0);
}
