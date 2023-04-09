#define LOG2 1.442695

const float transition = 0.025;
const float inv_transition = 1.0 / transition;
const float sample_height = 0.01;

vec4 c = texture(hdr, uv);
float d = rgba_to_depth(texture(depth, uv));
float d_norm = d / u_far;
float d_sqr = d_norm * d_norm;
vec4 fog_color = texture(atmos, vec2(uv.x, u_horizon + sample_height));

vec3 viewPos = WorldPosFromDepth(uv, d_norm);
float intensity;

if (d_norm > 1.0) {
    // intensity = (max((u_horizon + transition) - uv.y, 0.0) * inv_transition);
    // intensity = min(intensity, 1.0);
    intensity = 0;
} else {
    // intensity = 1.0 - exp2(-d_sqr * u_density * u_density * LOG2);
    intensity = fogIntensity(viewPos.y, d_norm);
}

// frag_color = mix(c, fog_color, intensity);
// frag_color = vec4(intensity, intensity, intensity, 1.0);
frag_color = vec4(viewPos, 1.0);
// frag_color = vec4(d_norm, d_norm, d_norm, 1.0);
// frag_color = c;
