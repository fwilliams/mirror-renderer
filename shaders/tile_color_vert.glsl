#pragma include "stddefs.glsl"

uniform mat4 reprojMat;

layout(location=0) in vec4 position;
layout(location=1) in vec3 texcoord;

flat out float v_texindex;

void main() {
	gl_Position = std_Projection * std_Modelview *  position;
	v_texindex = texcoord.z;
}