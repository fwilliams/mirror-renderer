#pragma include "stddefs.glsl"
#pragma include "stdutils.glsl"

uniform Material mat;

smooth in vec4 v_position;
smooth in vec3 v_normal;
smooth in vec2 v_texcoord;

out vec4 fragcolor;

void main() {
	vec3 color = std_GlobalAmbient.rgb;
			
	vec3 diffuse, specular;
	float attenuation;

	vec3 normal = normalize(v_normal);
	vec3 dirToViewer = normalize(-v_position.xyz);
	
	float n_dot_v = dot(normal, dirToViewer);
	float n_dot_v_clamped = max(n_dot_v, 0.0);
	
	for(int i = 0; i < std_Lights.length(); i++) {
		vec4 viewSpaceLightPos = std_View * std_Lights[i].position;
		vec3 dirToLight = normalize(vec3(viewSpaceLightPos - v_position));
		vec3 halfVec = normalize(dirToLight + dirToViewer);

		float n_dot_l = dot(normal, dirToLight);
		float n_dot_h = max(dot(normal, halfVec), 0.0);
		float v_dot_h = max(dot(dirToViewer, halfVec), 0.0);
		float l_dot_h = max(dot(halfVec, dirToLight), 0.0);
		float n_dot_l_clamped = max(n_dot_l, 0.0);
		
		float G = 2.0 * min(n_dot_v, n_dot_l) * n_dot_h / v_dot_h;
		      G = min(1.0, G);
		    
		float Fs = mat.reflectance + (1.0 - mat.reflectance) * pow((1.0 - l_dot_h), 5.0);
		float Fd = mat.reflectance + (1.0 - mat.reflectance) * pow((1.0 - n_dot_l_clamped), 5.0);
		
		float D = pow(n_dot_h, mat.roughness) * ((mat.roughness + 2)/(2*STD_PI));
		
		float specular_brdf = (Fs * D * G) / (4 * n_dot_l * n_dot_v);
		specular = n_dot_l_clamped * specular_brdf * mat.specular.rgb;
		diffuse = n_dot_l_clamped * (1.0 - Fd) * mat.diffuse.rgb;

		std_Attenuate(viewSpaceLightPos, std_Lights[i].attenuation, v_position, attenuation);
		color += attenuation * (diffuse + specular) * std_Lights[i].color.rgb;
	}
	
	fragcolor = vec4(color, 1.0);
	vec4 gamma = vec4(1.0/2.2);
	gamma.a = 1.0;
	fragcolor = pow(fragcolor, gamma);
}