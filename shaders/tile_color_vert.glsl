#pragma include "stddefs.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec3 texcoord;
layout(location=2) in vec3 mirrorCtr;

out vec2 v_texcoord;
flat out uint v_texindex;
flat out vec3 v_mirrorCtr;

void main() {
	gl_Position = std_Projection * std_Modelview *  position;
	v_mirrorCtr = normalize(mirrorCtr);

	v_texcoord = texcoord.xy;
	v_texindex = uint(texcoord.z);
}