uniform int numLayers;

uniform sampler2DArray imageTexArray;
uniform vec2 viewportSize;

out vec4 fragcolor;

void main() {
	fragcolor = vec4(vec3(0.0), 1.0);
	
	vec2 texcoord = gl_FragCoord.xy / viewportSize;
	texcoord.y = 1.0 - texcoord.y;
	
	for(int i = 0; i < numLayers; i++) {
		vec4 color = texture(imageTexArray, vec3(texcoord, i));
		if(fragcolor.xyz == vec3(0.0)) {
			fragcolor = color;
		} else {
			break;
		}
	}
}
