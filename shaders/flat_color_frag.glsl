#version 430

uniform sampler2DArray texid;

in vec2 v_texcoord; 
flat in uint v_texindex;
 
out vec4 fragcolor;

void main() {
	fragcolor = texture(texid, vec3(v_texcoord, v_texindex)); 
}