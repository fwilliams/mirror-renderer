#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;

in vec4 v_position;
in vec2 v_texcoord; 
flat in uint v_texindex;

out vec4 fragcolor;

uniform mat4 reprojMat;

void main() {
	float depth = texture(depthId, vec3(v_texcoord, v_texindex)).r * 50.0;
	vec2 uv = v_texcoord - vec2(0.5);
	vec3 pos = normalize(vec3(gl_FragCoord.xy, -0.5)) * depth;
	
	vec4 reprojectedPos = reprojMat * vec4(pos, 1.0);
	vec3 pp = reprojectedPos.xyz / reprojectedPos.w;
	vec2 tc = pp.xy/pp.z;
	
	/*
	fragcolor = texture(texid, vec3(tc, v_texindex));
	   
	fragcolor = vec4(tc, 0.0, 0.5);
	*/       
	fragcolor = 
	  vec4(normalize(pos+vec3(50.0)), 
	       texture(texid, vec3(v_texcoord, v_texindex)).a);
}
