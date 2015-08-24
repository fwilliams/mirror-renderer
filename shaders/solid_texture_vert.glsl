#pragma include "stddefs.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec3 texcoord;
layout(location=2) in vec3 mirrorCtr;

out vec2 v_texcoord;
out vec4 v_position;
flat out uint v_texindex;
flat out vec3 v_mirrorCtr;

void main() {
	gl_Position = std_Projection * std_Modelview * position;
	v_texcoord = texcoord.xy;
	v_texindex = uint(texcoord.z);
	v_position = position;
	v_mirrorCtr = mirrorCtr;
}