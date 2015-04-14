#version 430

struct Light {
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

layout(std140, binding=2) uniform PerFrameLightingBlock {
	vec4 global_ambient;
	Light lights[10];
};

layout(location = 1) uniform uint light_id;

out vec4 fragcolor;
void main() {
	fragcolor = max(lights[light_id].diffuse, lights[light_id].specular);
} 