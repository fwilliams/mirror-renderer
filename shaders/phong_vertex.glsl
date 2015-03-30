#version 430

layout(std140, binding=1) uniform MatrixBlock {
	mat4 modelview;
	mat4 projection;
	mat4 normal;
} matrices;

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

smooth out vec4 v_position;
smooth out vec3 v_normal;
smooth out vec2 v_texcoord;

void main() {
	v_position = matrices.modelview * in_position;
	v_normal = mat3(matrices.normal) * in_normal;
	v_texcoord = in_texcoord;
	gl_Position =  matrices.projection * matrices.modelview * in_position;
}