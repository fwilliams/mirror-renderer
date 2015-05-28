#version 330

#pragma include "stddefs.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec3 texcoord;

out vec2 v_texcoord;
flat out uint v_texindex;

void main() {
	gl_Position = std_Projection * std_Modelview * position;
	v_texcoord = texcoord.xy;
	v_texindex = uint(texcoord.z);
}