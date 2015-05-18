#version 430

flat in vec4 vColor;

out vec4 fragcolor;

void main() {
	fragcolor = vColor;
}