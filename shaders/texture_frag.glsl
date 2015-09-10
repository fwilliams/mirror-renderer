uniform sampler2D tex;

uniform vec2 viewportSize;

in vec4 gl_FragCoord;
out vec4 fragcolor;

void main() {
	vec2 texcoord = gl_FragCoord.xy / viewportSize;
	fragcolor = texture(tex, texcoord);
}