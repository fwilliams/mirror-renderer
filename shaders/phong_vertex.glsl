#version 430

layout(location = 0) uniform mat4 modelview_mat;
layout(location = 1) uniform mat4 proj_mat;
layout(location = 2) uniform mat3 normal_mat;

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

smooth out vec4 v_position;
smooth out vec3 v_normal;
smooth out vec2 v_texcoord;

void main() {
	v_position = modelview_mat * in_position;
	v_normal = normal_mat * in_normal;
	v_texcoord = in_texcoord;
	gl_Position =  proj_mat * modelview_mat * in_position;
}