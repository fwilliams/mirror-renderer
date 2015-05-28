#include "renderer.h"

#include <string.h>
#include <GL/glew.h>

#include <string>
#include <iostream>
#include <glm/gtx/string_cast.hpp>

using namespace glm;
using namespace std;

namespace renderer {

template<>
Renderer<GlVersion::GL430>::Renderer() {
  // Setup UBO for transformations
  const GLsizeiptr sizeofLights = NUM_LIGHTS * sizeof(Light);
  const GLsizeiptr sizeofGlobalAmb = sizeof(vec4);
  const GLsizeiptr sizeofMatrices = 4 * sizeof(mat4);

  glGenBuffers(1, &perFrameUbo);
  glBindBuffer(GL_UNIFORM_BUFFER, perFrameUbo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), nullptr, GL_DYNAMIC_DRAW);
  glBindBufferRange(GL_UNIFORM_BUFFER, MATS_UBO_BINDING_POINT, perFrameUbo, 0, sizeofMatrices);
  glBindBufferRange(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING_POINT, perFrameUbo, sizeofMatrices,
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
  programBuilder.addIncludeDir("shaders/glsl430");
}

template<>
Renderer<GlVersion::GL330>::Renderer() {
  programBuilder.addIncludeDir("shaders/glsl330");
}

template<>
Renderer<GlVersion::GL430>::~Renderer() {
  glDeleteBuffers(1, &perFrameUbo);
}

template<>
Renderer<GlVersion::GL330>::~Renderer() {
}

template <>
GLuint Renderer<GlVersion::GL430>::makeComputeProgramFromFile(const std::string& shader) {
  return programBuilder.buildComputeProgramFromFile(shader);
}

template <>
GLuint Renderer<GlVersion::GL430>::makeComputeProgramFromString(const std::string& shader) {
  return programBuilder.buildComputeProgramFromString(shader);
}

template<>
void Renderer<GlVersion::GL430>::startFrame() {
  glBindBuffer(GL_UNIFORM_BUFFER, perFrameUbo);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(PerFrameData), &perFrameData, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

template<>
void Renderer<GlVersion::GL330>::startFrame() {
}

template <>
void Renderer<GlVersion::GL330>::setupUniforms() {
  glUniformMatrix4fv(
      glGetUniformLocation(currentProgram, MV_MAT_UNIFORM_NAME),
      1, GL_FALSE, value_ptr(perFrameData.modelview_matrix));
  glUniformMatrix4fv(
      glGetUniformLocation(currentProgram, NORMAL_MAT_UNIFORM_NAME),
      1, GL_FALSE, value_ptr(perFrameData.normal_matrix));
  glUniformMatrix4fv(
      glGetUniformLocation(currentProgram, VIEW_MAT_UNIFORM_NAME),
      1, GL_FALSE, value_ptr(perFrameData.view_matrix));
  glUniformMatrix4fv(
      glGetUniformLocation(currentProgram, PROJ_MAT_UNIFORM_NAME),
      1, GL_FALSE, value_ptr(perFrameData.proj_matrix));
  glUniform4fv(
      glGetUniformLocation(currentProgram, GLOB_AMB_UNIFORM_NAME),
      1, value_ptr(perFrameData.global_ambient));

  for(size_t i = 0; i < NUM_LIGHTS; i++) {
    const string baseUniformName = string("std_Lights[") + std::to_string(i) + string("]");
    const string posName = baseUniformName + string(".position");
    const string diffName = baseUniformName + string(".diffuse");
    const string specName = baseUniformName + string(".specular");
    const string attName = baseUniformName + string(".attenuation");
    const string enbName = baseUniformName + string(".enabled");

    glUniform4fv(
        glGetUniformLocation(currentProgram, posName.c_str()),
        1, value_ptr(perFrameData.lights[i].pos));
    glUniform4fv(
        glGetUniformLocation(currentProgram, diffName.c_str()),
        1, value_ptr(perFrameData.lights[i].diffuse));
    glUniform4fv(
        glGetUniformLocation(currentProgram, specName.c_str()),
        1, value_ptr(perFrameData.lights[i].specular));
    glUniform1fv(
        glGetUniformLocation(currentProgram, attName.c_str()),
        1, &perFrameData.lights[i].attenuation);
    glUniform1fv(
        glGetUniformLocation(currentProgram, enbName.c_str()),
        1, &perFrameData.lights[i].enabled);
  }
}

template<>
void Renderer<GlVersion::GL330>::draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& pType) {
  perFrameData.modelview_matrix = view() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));

  setupUniforms();

  glBindVertexArray(geometry.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
  glDrawElements(pType, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

template<>
void Renderer<GlVersion::GL330>::draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& pType) {
  perFrameData.modelview_matrix = view() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));

  setupUniforms();

  glBindVertexArray(vao);
  glDrawArrays(pType, 0, num_vertices);
  glBindVertexArray(0);
}

template<>
void Renderer<GlVersion::GL430>::draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& pType) {
  glBindBuffer(GL_UNIFORM_BUFFER, perFrameUbo);
  perFrameData.modelview_matrix = view() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(PerFrameData, modelview_matrix), 2*sizeof(mat4),
                  value_ptr(perFrameData.modelview_matrix));

  glBindVertexArray(vao);
  glDrawArrays(pType, 0, num_vertices);
  glBindVertexArray(0);
}

template<>
void Renderer<GlVersion::GL430>::draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& pType) {
  glBindBuffer(GL_UNIFORM_BUFFER, perFrameUbo);
  perFrameData.modelview_matrix = view() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));
  glBufferSubData(GL_UNIFORM_BUFFER, offsetof(PerFrameData, modelview_matrix), 2*sizeof(mat4),
                  value_ptr(perFrameData.modelview_matrix));

  glBindVertexArray(geometry.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
  glDrawElements(pType, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

template<>
void Renderer<GlVersion::GL430>::setProgram(GLuint program) {
  glUseProgram(program);
}

template<>
void Renderer<GlVersion::GL330>::setProgram(GLuint program) {
  currentProgram = program;
  glUseProgram(program);
}

}
