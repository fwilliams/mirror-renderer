#pragma include "shaders/stddefs.glsl"

in vec4 in_position;

smooth out float interp_factor;

void main() {
	gl_Position =  projection * modelview * vec4(in_position.xyz, 1.0);
	interp_factor = in_position.w;
}