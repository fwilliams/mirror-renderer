#version 430

#pragma include "shaders/stddefs.glsl"
#pragma include "shaders/stdlighting.glsl"

layout(location = 3) uniform Material material;

smooth in vec4 v_position;
smooth in vec3 v_normal;
smooth in vec2 v_texcoord;

out vec4 fragcolor;

void main() {
	fragcolor = std_GlobalAmbient;
			
	vec4 diffuse, specular;
	vec3 normal = normalize(v_normal);
	for(uint i = 0; i < std_Lights.length(); i++) {
		vec3 dirToLight = normalize(vec3((std_Modelview * std_Lights[i].position) - v_position));
		vec3 dirToViewer = normalize(-v_position.xyz);

		std_Diffuse(dirToLight, normal, material, diffuse);
		std_BlinnPhongSpecular(dirToLight, dirToViewer, normal, material, specular);
		
		fragcolor += diffuse * std_Lights[i].diffuse + specular * std_Lights[i].specular;
	}
}