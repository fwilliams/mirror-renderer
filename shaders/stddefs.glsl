#version 430

struct Light {
	vec4 position;
	vec4 diffuse;
	vec4 specular;
	float attenuation;
	bool enabled;
};

struct Material {
	vec4 diffuse;
	vec4 specular;
	float shine_exp;
};

#define MATRIX_BLOCK_BINDING_POINT 1
#define PER_FRAME_LIGHT_BLOCK 2

layout(std140, binding=MATRIX_BLOCK_BINDING_POINT) uniform PerFrameMatrixBlock {
	mat4 modelview;
	mat4 projection;
	mat4 normal;
};

layout(std140, binding=PER_FRAME_LIGHT_BLOCK) uniform PerFrameLightingBlock {
	vec4 global_ambient;
	Light lights[10];
};
