#pragma include "shaders/stddefs.glsl"

layout(location = 1) uniform uint light_id;

out vec4 fragcolor;

void main() {
	fragcolor = max(lights[light_id].diffuse, lights[light_id].specular) + global_ambient;
}
