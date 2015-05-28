#extension GL_ARB_explicit_uniform_location : enable

#pragma include "stddefs.glsl"


layout(location = 1) uniform uint light_id;

out vec4 fragcolor;

void main() {
	fragcolor = max(std_Lights[light_id].diffuse, std_Lights[light_id].specular) + std_GlobalAmbient;
}
