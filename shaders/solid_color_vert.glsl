#pragma include "stddefs.glsl"

layout(location=0) in vec4 position;

void main() {
  gl_Position = std_Projection * std_Modelview * position;
}