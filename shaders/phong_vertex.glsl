#pragma include "stddefs.glsl"

in vec4 in_position;
in vec3 in_normal;
in vec2 in_texcoord;

smooth out vec4 v_position;
smooth out vec3 v_normal;
smooth out vec2 v_texcoord;

void main() {
	v_position = std_Modelview * in_position;
	v_normal = mat3(std_Normal) * in_normal;
	v_texcoord = in_texcoord;
	gl_Position =  std_Projection * std_Modelview * in_position;
}
