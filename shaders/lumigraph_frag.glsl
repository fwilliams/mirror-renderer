#pragma include "stddefs.glsl"

out vec4 fragcoord;

void main() {
	vec4 position = vec4(gl_FragCoord.xy - vec2(0.5), gl_FragCoord.z, 1.0);
	position = std_ModelView * fragcoord;
	vec2 st_plane_coords = 
}