#version 430

layout(std140, binding=1) uniform PerFrameMatrixBlock {
	mat4 modelview;
	mat4 projection;
	mat4 normal;
};

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

void main() {
	gl_Position =  projection * modelview * in_position;
}