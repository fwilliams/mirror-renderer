#version 430

#pragma include "shaders/stddefs.glsl"

layout(location=0) in vec4 position;
layout(location=1) in vec2 texcoord;

out vec2 v_texcoord;
 
void main() {
	gl_Position = std_Projection * std_Modelview * position;
	v_texcoord = texcoord;
}