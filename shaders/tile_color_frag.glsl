#pragma include "stddefs.glsl"

flat in float v_texindex;

out vec4 fragcolor;

void main() {	
	gl_FragDepth = (1.0f - gl_FragCoord.z);
	fragcolor = vec4(vec3(0.1, v_texindex, 0.1), 1.0);
}
