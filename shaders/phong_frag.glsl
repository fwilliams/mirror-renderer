#pragma include "stddefs.glsl"
#pragma include "stdutils.glsl"

uniform Material mat;

smooth in vec4 v_position;
smooth in vec3 v_normal;
smooth in vec2 v_texcoord;

out vec4 fragcolor;

void main() {
	fragcolor = std_GlobalAmbient;
			
	vec3 diffuse, specular, color;
	float attenuation;
	
	vec3 normal = normalize(v_normal);
	vec3 dirToViewer = normalize(-v_position.xyz);
	
	for(int i = 0; i < std_Lights.length(); i++) {
		vec4 viewSpaceLightPos = std_View * std_Lights[i].position;
		vec3 dirToLight = normalize(vec3(viewSpaceLightPos - v_position));
		vec3 halfVec = normalize(dirToLight + dirToViewer);
		
		float n_dot_l = max(dot(dirToLight, normal), 0.0);
		float h_dot_n = max(dot(halfVec, normal), 0.0);
			
		diffuse = n_dot_l * mat.diffuse.rgb * (1.0 - mat.reflectance);
		specular = pow(h_dot_n, mat.roughness) * 
		           ((mat.roughness + 2)/(2*STD_PI)) * 
		           mat.specular.rgb * mat.reflectance;
			
		std_Attenuate(viewSpaceLightPos, std_Lights[i].attenuation, v_position, attenuation);
			
		color += attenuation * (diffuse + specular) * std_Lights[i].color.rgb;
	}
	
	fragcolor = vec4(color, 1.0);
	vec4 gamma = vec4(1.0/2.2);
	gamma.a = 1.0;
	fragcolor = pow(fragcolor, gamma);
}