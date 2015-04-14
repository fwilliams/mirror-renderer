#version 430

#pragma include "stddefs.glsl"

layout(location = 0) uniform mat4 modelview_mat;
layout(location = 1) uniform mat4 proj_mat;

layout(std140, binding=1) uniform MatrixBlock {
	mat4 modelview;
	mat4 projection;
	mat4 normal;
} matrices;

in vec4 in_position;

smooth out float interp_factor;

void main() {
	gl_Position =  matrices.projection * matrices.modelview * vec4(in_position.xyz, 1.0);
	interp_factor = in_position.w;
}