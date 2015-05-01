#pragma include "shaders/stddefs.glsl"

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

void main() {
	gl_Position =  projection * modelview * in_position;
}