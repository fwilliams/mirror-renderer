#version 430

uniform sampler2D texid;

in vec2 v_texcoord; 
 
out vec4 fragcolor;

void main() {
	fragcolor = texture(texid, v_texcoord); 
}