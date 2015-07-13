#pragma include "stddefs.glsl"
#pragma include "stdlighting.glsl"


uniform Material material;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

smooth in vec4 v_position;
smooth in vec3 v_normal;
smooth in vec2 v_texcoord;

out vec4 fragcolor;

void main() {
	fragcolor = std_GlobalAmbient;
			
	vec4 diffuse, specular;
	float attenuation;
	
	vec3 normal = normalize(v_normal);
	for(int i = 0; i < std_Lights.length(); i++) {
		//if(std_Lights[i].enabled > 0.5) {
			vec4 viewSpaceLightPos = std_View * std_Lights[i].position;
			vec3 dirToLight = normalize(vec3(viewSpaceLightPos - v_position));
			vec3 dirToViewer = normalize(-v_position.xyz);
	
			std_Diffuse(dirToLight, normal, material, diffuse);
			std_BlinnPhongSpecular(dirToLight, dirToViewer, normal, material, specular);
			std_Attenuation(viewSpaceLightPos, std_Lights[i].attenuation, v_position, attenuation);
			
			fragcolor += attenuation * (diffuse * std_Lights[i].diffuse + specular * std_Lights[i].specular);
		//}
	}
	
	vec4 gamma = vec4(1.0/2.2);
	gamma.a = 1.0;
	fragcolor = pow(fragcolor, gamma);
}