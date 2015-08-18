#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;

uniform uint numMirrorFaces;

in vec4 v_position;
in vec2 v_texcoord; 
flat in uint v_texindex;

out vec4 fragcolor;

uniform mat4 magicmat;

void main() {
	float depth = 
	  texture(depthId, vec3(v_texcoord, v_texindex)).x * 100.0;
	vec2 pos_xy = depth * (v_texcoord - vec2(0.5)) / 0.5;
	vec4 pos = magicmat * vec4(pos_xy, depth, 1.0);
	vec2 texcoords = (pos.xy / pos.w);
	texcoords = vec2(1.0) - (vec2(0.5) + texcoords);
	fragcolor = texture(texid, vec3(texcoords, v_texindex)); 
	
	gl_FragDepth = (1.0f - gl_FragCoord.z);
	
	float ratio = v_texindex;
	ratio = ratio / numMirrorFaces;
	fragcolor = vec4(vec3(ratio), 1.0);
}
