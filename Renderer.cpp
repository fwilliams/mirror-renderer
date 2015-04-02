#include "Renderer.h"

#include <string.h>

#include "utils/gl_program_builder.h"

using namespace glm;

Renderer::Renderer() {
  // Setup UBO for transformations
  glGenBuffers(1, &matrices_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, MATS_UBO_BINDING_POINT, matrices_ubo);

  mat4* matrices = reinterpret_cast<mat4*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  matrices[0] = mat4(1.0);
  matrices[1] = mat4(1.0);
  matrices[2] = mat4(1.0);
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  // Setup the UBO for lights
  glGenBuffers(1, &lights_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
  glBufferData(GL_UNIFORM_BUFFER, NUM_LIGHTS * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING_POINT, lights_ubo);

  Light* lights = reinterpret_cast<Light*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  memset(lights, 0, NUM_LIGHTS * sizeof(Light));
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  draw_normals_program = ProgramBuilder::buildFromFiles("shaders/draw_normals_vert.glsl",
                                                      "shaders/draw_normals_frag.glsl");
}

Renderer::~Renderer() {
  glDeleteBuffers(1, &matrices_ubo);
  glDeleteBuffers(1, &lights_ubo);
  glDeleteProgram(draw_normals_program);
  // TODO Auto-generated destructor stub
}

