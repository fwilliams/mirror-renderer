#pragma include "stddefs.glsl"

in vec4 in_position;

smooth out float interp_factor;

void main() {
	gl_Position =  std_Projection * std_Modelview * vec4(in_position.xyz, 1.0);
	interp_factor = in_position.w;
}
