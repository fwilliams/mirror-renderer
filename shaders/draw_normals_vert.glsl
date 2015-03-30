#version 430

layout(location = 0) uniform mat4 modelview_mat;
layout(location = 1) uniform mat4 proj_mat;

in vec4 in_position;

smooth out float interp_factor;

void main() {
	gl_Position =  proj_mat * modelview_mat * vec4(in_position.xyz, 1.0);
	interp_factor = in_position.w;
}