// Depth (range = 0 .. u_far)
float centerDepth = getDepth( uv );

// Depth (range = 0 .. 1)
float centerDepthNorm = centerDepth / u_far;
float centerViewZ = getViewZ( u_near, u_far, centerDepth );
vec3 viewPosition = getViewPosition( uv, centerDepthNorm, centerViewZ, u_mat_p, u_inv_mat_p );
float ambientOcclusion = getAmbientOcclusion( viewPosition, centerDepth );

// Store value as rgba to increase precision
float max_dist = 0.1;
float mult = 1.0 / max_dist;
frag_color = float_to_rgba(ambientOcclusion) * max(0.0, max_dist - centerDepthNorm) * mult;
