#pragma include "stddefs.glsl"

uniform sampler2DArray texid;
uniform sampler2DArray depthId;
uniform sampler2D vcTex;

uniform vec2 viewportSize;
uniform vec3 cameraPos;
uniform float np;

in vec4 v_position;
in vec2 v_texcoord; 
flat in uint v_texindex;
flat in vec3 v_mirrorCtr;

out vec4 fragcolor;

void main() {
	fragcolor = texture(vcTex, v_texcoord);
}

	/*
	float f = length(v_mirrorCtr.z);

	float depth = texture(depthId, vec3(v_texcoord, v_texindex)).r * 50.0;
	vec2 uvPos = gl_FragCoord.xy/viewportSize - vec2(0.5);
	vec3 pos = normalize(vec3(uvPos, 1.0/(f - 0.5) )) * depth;
	
	mat4 s = mat4(vec4(1.0/f, 0, 0, 0),
	     vec4(0, 1.0/f, 0, 0),
	     vec4(0, 0, 1.0/f, 0),
	     vec4(0, 0, 0, 1.0/f));
	float fMinusZc = f - cameraPos.z;
	mat4 rpMat =  mat4(vec4(fMinusZc, 0, 0, 0), 
	                  vec4(0, fMinusZc, 0, 0),
	                  vec4(cameraPos.x, cameraPos.y, f, 1),
	                  vec4(-f*cameraPos, -cameraPos.z));

    vec4 reprojectedPos = rpMat * vec4(pos, 1.0);
    vec3 pp = reprojectedPos.xyz / reprojectedPos.w;
    vec2 tc = pp.xy/pp.z + vec2(0.5);	

	fragcolor = texture(texid, vec3(tc, v_texindex));
	*/
	/*	   
	float alpha = texture(texid, vec3(v_texcoord, v_texindex)).a;
	fragcolor = vec4(v_mirrorCtr.rgb, alpha);
	fragcolor = vec4(tc, 0.0, 0.5);
	fragcolor = 
	  vec4(normalize(pos+vec3(50.0)), 
	       texture(texid, vec3(v_texcoord, v_texindex)).a);
	*/    