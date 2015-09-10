uniform int numLayers;

uniform sampler2DArray imageTexArray;
uniform sampler2DArray depthTexArray;

uniform vec2 viewportSize;
uniform float blendCoeff;

uniform bool flip;

layout(origin_upper_left) in vec4 gl_FragCoord;

layout(location = 0) out vec4 fragcolor;

void main() {
	fragcolor = vec4(vec3(0.0), 1.0);
	float pixelDepth = -1.0;
	
	vec2 texcoord = gl_FragCoord.xy / viewportSize;
	if(flip) {
		texcoord.y = 1.0 - texcoord.y;
	}
	
	for(int i = 0; i < numLayers; i++) {
		vec4 color = texture(imageTexArray, vec3(texcoord, i));
		float depth = texture(depthTexArray, vec3(texcoord, i)).x;
		
		// 3 cases:
		// 1) fragcolor == 0                       => choose pixel from texture
		// 2) fragcolor != 0 && depths match       => blend fragclor with texture
		// 3) fragcolor != 0 && depths don't match => choose pixel with closer depth
		if(color.rgb != vec3(0.0)) {
			if(fragcolor.rgb == vec3(0.0)) {
				fragcolor.rgb = color.rgb;
				pixelDepth = depth;
			} else if(pixelDepth == depth) { // The pixels correspond to the same point
				fragcolor.rgb = mix(color.xyz, fragcolor.rgb, blendCoeff);
			} else if(depth < pixelDepth) {  // The pixels correspond to different points and the new one is in front
				fragcolor.rgb = color.rgb;// + vec3(0.5, 0.0, 0.0);
				pixelDepth = depth;
			}
		}
	}
}
