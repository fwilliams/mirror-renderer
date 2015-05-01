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
	float attenuation;
	
	vec3 normal = normalize(v_normal);
	for(uint i = 0; i < std_Lights.length(); i++) {
		vec4 viewSpaceLightPos = std_Modelview * std_Lights[i].position;
		vec3 dirToLight = normalize(vec3(viewSpaceLightPos - v_position));
		vec3 dirToViewer = normalize(-v_position.xyz);

		std_Diffuse(dirToLight, normal, material, diffuse);
		std_BlinnPhongSpecular(dirToLight, dirToViewer, normal, material, specular);
		std_Attenuation(viewSpaceLightPos, std_Lights[i].attenuation, v_position, attenuation);
		
		fragcolor += attenuation * (diffuse * std_Lights[i].diffuse + specular * std_Lights[i].specular);
	}
	
	vec4 gamma = vec4(1.0/1.8);
	gamma.a = fragcolor.a;
	fragcolor = pow(fragcolor, gamma);
}