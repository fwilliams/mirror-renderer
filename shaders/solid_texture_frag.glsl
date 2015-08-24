#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;
uniform sampler2D vcTex;

uniform vec2 viewportSize;
uniform vec3 cameraPos;

in vec4 v_position;
in vec2 v_texcoord; 
flat in uint v_texindex;
flat in vec3 v_mirrorCtr;

out vec4 fragcolor;

void main() {
	vec2 uv = gl_FragCoord.xy / viewportSize;
	float depth = texture(vcTex, uv).r*50.0;
	
	vec2 uvPos = uv - vec2(0.5);
	vec3 pos = normalize(vec3(uvPos, -1.0)) * depth;
	
	
	float f = length(v_mirrorCtr);
	float fMinusZc = f - cameraPos.z;
	mat4 rpMat = mat4(vec4(fMinusZc, 0, 0, 0), 
	                  vec4(0, fMinusZc, 0, 0),
	                  vec4(cameraPos.x, cameraPos.y, f, 1),
	                  vec4(-f*cameraPos, -cameraPos.z));
    vec4 reprojectedPos = rpMat * vec4(pos, 1.0);
    vec3 pp = reprojectedPos.xyz / reprojectedPos.w;
    vec2 tc = pp.xy/pp.z + vec2(0.5);	
                   
	fragcolor = texture(texid, vec3(tc, v_texindex));
}