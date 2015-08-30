#pragma include "stddefs.glsl"

uniform sampler2DArray imageTexArray;
uniform sampler2DArray depthTexArray;

uniform vec2 viewportSize;
uniform vec3 cameraPos;
uniform float f;
uniform uint texindex;

out vec4 fragcolor;

void main() {
	vec2 uv = gl_FragCoord.xy / viewportSize;
	uv = vec2(uv.x, 1.0 - uv.y);
	float depth = texture(depthTexArray, vec3(uv, texindex)).r * 5.0;
	
	vec2 uvPos = uv - vec2(0.5);
	vec3 pos = normalize(vec3(uvPos, 1.0)) * depth;
	
	float fMinusZc = f - cameraPos.z;
	mat4 rpMat = mat4(vec4(fMinusZc, 0, 0, 0), 
	                  vec4(0, fMinusZc, 0, 0),
	                  vec4(cameraPos.x, cameraPos.y, f, 1),
	                  vec4(-f*cameraPos, -cameraPos.z));
    vec4 reprojectedPos = rpMat * vec4(pos, 1.0);
    vec3 pp = reprojectedPos.xyz / reprojectedPos.w;
    vec2 tc = pp.xy/pp.z + vec2(0.5);

	fragcolor = texture(depthTexArray, vec3(uv, texindex));
	fragcolor = texture(imageTexArray, vec3(uv, texindex));
	fragcolor = texture(imageTexArray, vec3(tc, texindex));
	fragcolor.xyz = normalize(pos);
}