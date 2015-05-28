uniform samplerCube skyboxSampler;

in vec3 v_position;

out vec4 fragcolor;

void main() {
	vec3 pos = v_position + vec3(0.5);
	fragcolor = vec4(normalize(pos), 1.0);//texture(skyboxSampler, v_position);
}w