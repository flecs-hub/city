#include "etc/sokol/shaders/common.glsl"
#include "etc/sokol/shaders/atmosphere.glsl"

const float a = 2.0;
const float b = 4.0;

vec3 WorldPosFromDepth(vec2 uv, float depth)
{
    // vec4 clip_space_position = vec4(uv * 2.0 - vec2(1.0), 2.0 * depth - 1.0, 1.0);
    vec4 clip_space_position = vec4(uv * 2.0 - vec2(1.0), depth, 1.0);
    vec4 position = u_inv_mat_vp * clip_space_position;
    return(position.xyz / position.w);
}

float fogIntensity(float rayY, float distance) {
    return (a / b) * exp(-rayY * b) * (1.0 - exp(-distance * rayY * b )) / rayY;
}
