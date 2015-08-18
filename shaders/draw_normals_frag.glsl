//#extension GL_ARB_explicit_uniform_location : enable

#extension GL_ARB_separate_shader_objects : enable

out vec4 fragcolor;

smooth in float interp_factor;

layout(location=2) uniform vec4 color1;
layout(location=3) uniform vec4 color2;

void main() {
	fragcolor = mix(color1, color2, interp_factor);
}