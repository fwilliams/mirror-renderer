#version 430

#pragma include "shaders/stddefs.glsl"

layout(location = 1) uniform uint light_id;

out vec4 fragcolor;

void main() {
	fragcolor = max(std_Lights[light_id].diffuse, std_Lights[light_id].specular) + std_GlobalAmbient;
}
