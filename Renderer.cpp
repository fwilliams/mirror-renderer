#include "Renderer.h"

#include <string.h>

#include "utils/gl_program_builder.h"

using namespace glm;

Renderer::Renderer() {
  glGenBuffers(1, &matrices_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(mat4) * 3, nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, MATS_UBO_BINDING_POINT, matrices_ubo);

  reinterpret_cast<mat4*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  *projection() = mat4(1.0);
  *view() = mat4(1.0);
  *normal() = mat4(1.0);

  glGenBuffers(1, &lights_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
  glBufferData(GL_UNIFORM_BUFFER, NUM_LIGHTS * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING_POINT, lights_ubo);
  Light* lights = reinterpret_cast<Light*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));

  memset(lights, 0, NUM_LIGHTS * sizeof(Light));

  draw_normals_program = ProgramBuilder::buildFromFiles("shaders/draw_normals_vert.glsl",
                                                      "shaders/draw_normals_frag.glsl");
}

Renderer::~Renderer() {
  // TODO Auto-generated destructor stub
}

