#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;

in vec2 v_texcoord; 
flat in uint v_texindex;
flat in vec3 v_mirrorCtr;

layout(location = 0) out vec4 fragcolor;

void main() {	
	vec3 tc = vec3(v_texcoord, v_texindex);
	fragcolor = vec4(texture(depthId, tc).rgb, texture(texid, tc).a); 
	fragcolor = texture(depthId, tc);
	
}
