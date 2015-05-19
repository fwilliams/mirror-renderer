#version 430

#pragma include "shaders/stddefs.glsl"

layout(location=1) in vec4 position;
layout(location=0) in vec4 color;

smooth out vec4 vColor;

void main() {
	gl_Position = std_Projection * std_Modelview * position;
	vColor = color;
}