const float STD_PI = 3.1415926535897932384626433832795;

void std_Attenuate(in vec4 lightPos, in float k, in vec4 pos, out float attenuation) {
	float distanceToLight = distance(lightPos, pos);
	attenuation = 1.0 / (1.0 + k * pow(distanceToLight, 2.0));
}