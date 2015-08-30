#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;

in vec2 v_texcoord; 
flat in uint v_texindex;

layout(location = 0) out vec4 fragcolor;

void main() {	
	vec3 tc = vec3(v_texcoord, v_texindex);
	fragcolor = texture(texid, tc); 
}
