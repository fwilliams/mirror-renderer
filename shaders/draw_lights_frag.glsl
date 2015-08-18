//#extension GL_ARB_explicit_uniform_location : enable
#extension GL_ARB_separate_shader_objects : enable

#pragma include "stddefs.glsl"


layout(location = 1) uniform uint light_id;

out vec4 fragcolor;

void main() {
	fragcolor = std_Lights[light_id].color + std_GlobalAmbient;
}
