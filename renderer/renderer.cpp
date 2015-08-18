#include <string.h>
#include <GL/glew.h>

#include <string>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "renderer.h"

using namespace glm;
using namespace std;
using namespace utils;

Renderer::Renderer() {
  programBuilder.addIncludeDir("shaders/glsl330");
  materialProgram = programBuilder.buildFromFiles(
		  "shaders/phong_vertex.glsl",
		  "shaders/physical_frag.glsl");

  drawLightsProgram = programBuilder.buildFromFiles(
		  "shaders/draw_lights_vert.glsl",
		  "shaders/draw_lights_frag.glsl");

  drawNormalsProgram = programBuilder.buildFromFiles(
		  "shaders/draw_normals_vert.glsl",
		  "shaders/draw_normals_frag.glsl");
}

Renderer::~Renderer() {
	glDeleteProgram(materialProgram);
	glDeleteProgram(drawLightsProgram);
	glDeleteProgram(drawNormalsProgram);
}

void Renderer::startFrame() {
}

void Renderer::setupUniforms() {
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
    const string colorName = baseUniformName + string(".color");
    const string attName = baseUniformName + string(".attenuation");
    const string enbName = baseUniformName + string(".enabled");

    glUniform4fv(
        glGetUniformLocation(currentProgram, posName.c_str()),
        1, value_ptr(perFrameData.lights[i].pos));
    glUniform4fv(
        glGetUniformLocation(currentProgram, colorName.c_str()),
        1, value_ptr(perFrameData.lights[i].color));
    glUniform1fv(
        glGetUniformLocation(currentProgram, attName.c_str()),
        1, &perFrameData.lights[i].attenuation);
    glUniform1fv(
        glGetUniformLocation(currentProgram, enbName.c_str()),
        1, &perFrameData.lights[i].enabled);
  }
}

void Renderer::draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& pType) {
  perFrameData.modelview_matrix = viewMatrix() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));

  setupUniforms();

  glBindVertexArray(geometry.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
  glDrawElements(pType, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& pType) {
  perFrameData.modelview_matrix = viewMatrix() * transform;
  perFrameData.normal_matrix = transpose(inverse(mat4(mat3(perFrameData.modelview_matrix))));

  setupUniforms();

  glBindVertexArray(vao);
  glDrawArrays(pType, 0, num_vertices);
  glBindVertexArray(0);
}

void Renderer::drawLights(const Geometry& g, const glm::mat4& transform, const PrimitiveType& p) {
	setProgram(drawLightsProgram);
	for(size_t i = 0; i < numLights(); i++) {
		glUniform1ui(1, i);
		draw(g, glm::translate(glm::mat4(1.0), lightPosition(i)) * transform, p);
	}
}

void Renderer::drawNormals(const Geometry& g, const glm::mat4& transform, const glm::vec4& color1, const glm::vec4& color2) {
	  static const GLuint COLOR1_LOC = 2;
	  static const GLuint COLOR2_LOC = 3;

	  setProgram(drawNormalsProgram);
	  glUniform4fv(COLOR1_LOC, 1, glm::value_ptr(color1));
	  glUniform4fv(COLOR2_LOC, 1, glm::value_ptr(color2));
	  draw(g.normal_view_vao, g.num_vertices * 2, transform, LINES);
}

void Renderer::setProgram(GLuint program) {
  currentProgram = program;
  glUseProgram(program);
}
