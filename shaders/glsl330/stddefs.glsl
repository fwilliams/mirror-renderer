struct Light {
    vec4 position;
    vec4 diffuse;
    vec4 specular;
    float attenuation;
    float enabled;
};

struct Material {
    vec4 diffuse;
    vec4 specular;
    float shine_exp;
};

uniform mat4 std_Modelview;
uniform mat4 std_Normal;
uniform mat4 std_View;
uniform mat4 std_Projection;
uniform vec4 std_GlobalAmbient;
uniform Light std_Lights[10];
