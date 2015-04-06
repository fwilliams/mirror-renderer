#version 430

struct Light {
	vec4 position;
	vec4 diffuse;
	vec4 specular;
};

struct Material {
	vec4 diffuse;
	vec4 specular;
	float shine_exp;
};


layout(std140, binding=1) uniform PerFrameMatrixBlock {
	mat4 modelview;
	mat4 projection;
	mat4 normal;
};

layout(std140, binding=2) uniform PerFrameLightingBlock {
	vec4 global_ambient;
	Light lights[10];
};

layout(location = 3) uniform Material material;

smooth in vec4 v_position;
smooth in vec3 v_normal;
smooth in vec2 v_texcoord;

out vec4 fragcolor;

void main() {
	fragcolor = global_ambient;

	vec3 normal = normalize(v_normal);
	for(uint i = 0; i < lights.length(); i++) {
		vec4 view_space_light_pos =  modelview * lights[i].position;
		vec3 dir_to_light = normalize(vec3(view_space_light_pos - v_position));
		vec3 dir_to_viewer = normalize(-v_position).xyz;
		// vec3 reflection_dir = normalize(reflect(-dir_to_light, normal));
		//float r_dot_v = clamp(dot(reflection_dir, dir_to_viewer), 0.0, 1.0);
		 
		vec3 h = normalize(dir_to_light + dir_to_viewer);
		float h_dot_n = clamp(dot(h, normal), 0.0, 1.0);		
		
		float diffuse = clamp(dot(dir_to_light, normal), 0.0, 1.0);		
		float specular = pow(h_dot_n, material.shine_exp);
		
		fragcolor += material.diffuse * diffuse * lights[i].diffuse + 
		             material.specular * specular * lights[i].specular;
	}
}