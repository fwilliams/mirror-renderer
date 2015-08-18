#pragma include "stddefs.glsl"

in vec4 position;

void main() {
  gl_Position = std_Projection * std_Modelview * position;
}
