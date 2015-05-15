void std_Diffuse(in vec3 dirToLight, in vec3 normal, in Material mat, out vec4 diffuse) {
	diffuse = max(dot(dirToLight, normal), 0.0) * mat.diffuse;
}

void std_PhongSpecular(in vec3 dirToLight, in vec3 dirToViewer, in vec3 normal, in Material mat, out vec4 specular) {
	vec3 reflectionDir = normalize(reflect(-dirToLight, normal));
	float rDotV = max(dot(reflectionDir, dirToViewer), 0.0);
	specular = pow(rDotV, mat.shine_exp) * mat.specular;
}

void std_BlinnPhongSpecular(in vec3 dirToLight, in vec3 dirToViewer, in vec3 normal, in Material mat, out vec4 specular) {
	vec3 h = normalize(dirToLight + dirToViewer);
	float hDotN = max(0.0, dot(h, normal));
	specular = pow(hDotN, mat.shine_exp) * mat.specular;
}

void std_Attenuation(in vec4 lightPos, in float k, in vec4 pos, out float attenuation) {
	float distanceToLight = distance(lightPos, pos);
	attenuation = 1.0 / (1.0 + k * pow(distanceToLight, 2.0));
}
