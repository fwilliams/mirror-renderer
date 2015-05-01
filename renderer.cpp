#include "renderer.h"

#include <string.h>
#include <GL/glew.h>

#include <iostream>
#include <glm/gtx/string_cast.hpp>

#include "utils/gl_program_builder.h"

using namespace glm;
using namespace std;

Renderer::Renderer() {
  // Setup UBO for transformations
  const GLsizeiptr sizeofLights = NUM_LIGHTS * sizeof(Light);
  const GLsizeiptr sizeofGlobalAmb = sizeof(vec4);
  const GLsizeiptr sizeofMatrices = 4 * sizeof(mat4);

  glGenBuffers(1, &per_frame_ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, MATS_UBO_BINDING_POINT, per_frame_ubo, 0, sizeofMatrices);
  glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING_POINT, per_frame_ubo, sizeofMatrices,
                    sizeofGlobalAmb + sizeofLights);

  // Write identity matrix for each matrix
  mat4* matrices = reinterpret_cast<mat4*>(
  glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeofMatrices, GL_MAP_WRITE_BIT));
  matrices[0] = mat4(1.0);
  matrices[1] = mat4(1.0);
  matrices[2] = mat4(1.0);
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  // Write empty global ambient vector
  vec4* ga = reinterpret_cast<vec4*>(glMapBufferRange(GL_UNIFORM_BUFFER, sizeofMatrices,
                                                      sizeofGlobalAmb, GL_MAP_WRITE_BIT));
  *ga = vec4(0.0, 0.0, 0.0, 1.0);
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  // Write 0 for all light data
  Light* lights = reinterpret_cast<Light*>(
  glMapBufferRange(GL_UNIFORM_BUFFER, sizeofMatrices + sizeofGlobalAmb, sizeofLights,
                   GL_MAP_WRITE_BIT));
  memset(lights, 0, NUM_LIGHTS * sizeof(Light));
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

Renderer::~Renderer() {
  glDeleteBuffers(1, &per_frame_ubo);
}

void Renderer::startFrame() {
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), &perFrameData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::draw(GLuint vao, size_t num_vertices, const glm::mat4& transform) {
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  PerFrameData* data = reinterpret_cast<PerFrameData*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  data->view_matrix = view() * transform;
  data->normal_matrix = glm::transpose(glm::inverse(glm::mat4(glm::mat3(data->view_matrix))));
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, num_vertices);
  glBindVertexArray(0);
}

void Renderer::draw(const Geometry& geometry, const glm::mat4& transform) {
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  PerFrameData* data = reinterpret_cast<PerFrameData*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
  data->view_matrix = view() * transform;
  data->normal_matrix = glm::transpose(glm::inverse(glm::mat4(glm::mat3(data->view_matrix))));
  glUnmapBuffer(GL_UNIFORM_BUFFER);

  glBindVertexArray(geometry.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
  glDrawElements(GL_TRIANGLES, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::setProgram(GLuint program) {
  glUseProgram(program);
}
