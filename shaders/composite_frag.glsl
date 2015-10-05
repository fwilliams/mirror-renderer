uniform int numLayers;

uniform sampler2DArray imageTexArray;
uniform sampler2DArray depthTexArray;
uniform sampler2D backgroundTex;

uniform vec2 viewportSize;
uniform float blendCoeff;

uniform float exponent;

uniform float kernelSize;
uniform float overlap;

uniform bool flip;

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) out vec4 fragcolor;

void main() {
	vec3 layerContributions[25];
	float depthContributions[25];
	
	fragcolor = vec4(vec3(0.0), 1.0);
	float pixelDepth = 1.0;
	
	vec2 texcoord = gl_FragCoord.xy / viewportSize;
	if(flip) {
		texcoord.y = 1.0 - texcoord.y;
	}
	
	for(int i = 0; i < numLayers; i++) {
		vec4 color = texture(imageTexArray, vec3(texcoord, i));
		float depth = texture(depthTexArray, vec3(texcoord, i)).x;
		
		layerContributions[i] = vec3(0.0);
		depthContributions[i] = 1.0;
		
		// 3 cases:
		// 1) fragcolor == 0                       => choose pixel from texture
		// 2) fragcolor != 0 && depths match       => blend fragclor with texture
		// 3) fragcolor != 0 && depths don't match => choose pixel with closer depth
		if(/*color.rgb != vec3(0.0) &&*/ depth != 1.0) {
			if(fragcolor.rgb == vec3(0.0)) { // The pixel is empty
				fragcolor.rgb = color.rgb;
				layerContributions[i] = color.rgb;
				depthContributions[i] = depth;
				pixelDepth = depth;
			} else if(pixelDepth == depth) { // The pixels correspond to the same point
				// TODO: compute blend coefficent from depth
				fragcolor.rgb = mix(color.xyz, fragcolor.rgb, blendCoeff);
				layerContributions[i] = color.rgb;
				depthContributions[i] = depth;
			} else if(depth < pixelDepth) {  // The pixels correspond to different points and the new one is in front
				fragcolor.rgb = color.rgb;// + vec3(0.5, 0.0, 0.0);
				layerContributions[i] = color.rgb;
				depthContributions[i] = depth;
				pixelDepth = depth;
			}
		}
	}
	
	float numContributingImgs = 0;
	fragcolor.rgb = vec3(0.0);
	for(int i = 0; i < numLayers; i++) {
		if(/*layerContributions[i] != vec3(0) &&*/ depthContributions[i] == pixelDepth && pixelDepth != 1) {
			numContributingImgs += 1;
			fragcolor.rgb += layerContributions[i];
		}
	}
	
	if(numContributingImgs != 0) {
		fragcolor.rgb /= numContributingImgs;
	}
		
	
	if(fragcolor.rgb == vec3(0)) {
		fragcolor.rgb = texture(backgroundTex, texcoord).rgb;
	}
	
	/*
	if(numContributingImgs == 1) {
		fragcolor.rgb = vec3(1, 0, 0);
	} else if(numContributingImgs == 2) {
		fragcolor.rgb = vec3(0, 1, 0);
	} else if(numContributingImgs == 3) {
		fragcolor.rgb = vec3(0, 0, 1);
	} else {
		fragcolor.rgb = vec3(1, 1, 0);
	}
	*/
	//fragcolor.rgb = vec3(pixelDepth);
}
