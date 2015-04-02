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

  // Make debug program which draws the normals of a piece of geometry
  draw_normals_program = ProgramBuilder::buildFromFiles("shaders/draw_normals_vert.glsl",
                                                        "shaders/draw_normals_frag.glsl");
}

Renderer::~Renderer() {
  glDeleteBuffers(1, &per_frame_ubo);
  glDeleteProgram(draw_normals_program);
}

void Renderer::startFrame() {
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), &perFrameData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::drawNormals(glm::vec4 baseColor, glm::vec4 tailColor, Geometry& geometry) {
  glUseProgram(draw_normals_program);
  glBindVertexArray(geometry.normal_view_vao);
  glUniform4fv(COLOR1_LOC, 1, glm::value_ptr(baseColor));
  glUniform4fv(COLOR2_LOC, 1, glm::value_ptr(tailColor));
  glDrawArrays(GL_LINES, 0, geometry.num_vertices * 2);
  glBindVertexArray(0);
  glUseProgram(0);
}

void Renderer::drawWireframe(GLuint program, Geometry& geometry) {
  glPolygonMode(GL_FRONT_FACE, GL_LINE);
  draw(program, geometry);
  glPolygonMode(GL_FRONT_FACE, GL_FILL);
}

void Renderer::draw(GLuint program, Geometry& geometry) {
  perFrameData.normal_matrix = glm::transpose(glm::inverse(glm::mat4(glm::mat3(view()))));
  glBindBuffer(GL_UNIFORM_BUFFER, per_frame_ubo);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::mat4),
      &perFrameData.normal_matrix);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  glUseProgram(program);
  glBindVertexArray(geometry.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
  glDrawElements(GL_TRIANGLES, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glUseProgram(0);
}
