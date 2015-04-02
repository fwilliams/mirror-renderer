#include <GL/glew.h>

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/geometry.h"

#ifndef ENGINE_H_
#define ENGINE_H_

struct Light {
  glm::vec4 pos;
  glm::vec4 diffuse;
  glm::vec4 specular;
};

class Renderer {
  GLuint matrices_ubo = 0;
  GLuint lights_ubo = 0;
  GLuint draw_normals_program = 0;

  const GLuint MATS_UBO_BINDING_POINT = 1;
  const GLuint LIGHTS_UBO_BINDING_POINT = 2;

  const GLuint COLOR1_LOC = 2;
  const GLuint COLOR2_LOC = 3;

  static const GLuint NUM_LIGHTS = 10;

  Light lights[NUM_LIGHTS];
  glm::mat4 matrices[3];

  glm::mat4* const proj_matrix = &matrices[1];
  glm::mat4* const view_matrix = &matrices[0];
  glm::mat4* const norm_matrix = &matrices[2];

public:
  Renderer();
  virtual ~Renderer();

  void startFrame() {
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*3, matrices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, lights_ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Light)*NUM_LIGHTS, lights, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void drawNormals(glm::vec4 baseColor, glm::vec4 tailColor, Geometry& geometry) {
    glUseProgram(draw_normals_program);
    glBindVertexArray(geometry.normal_view_vao);
    glUniform4fv(COLOR1_LOC, 1, glm::value_ptr(baseColor));
    glUniform4fv(COLOR2_LOC, 1, glm::value_ptr(tailColor));
    glDrawArrays(GL_LINES, 0, geometry.num_vertices * 2);
    glBindVertexArray(0);
    glUseProgram(0);
  }

  void drawWireframe(GLuint program, Geometry& geometry) {
    glPolygonMode(GL_FRONT_FACE, GL_LINE);
    draw(program, geometry);
    glPolygonMode(GL_FRONT_FACE, GL_FILL);
  }

  void draw(GLuint program, Geometry& geometry) {
    *norm_matrix = glm::transpose(glm::inverse(glm::mat4(glm::mat3(*view_matrix))));
    glBindBuffer(GL_UNIFORM_BUFFER, matrices_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4)*2, sizeof(glm::mat4), norm_matrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUseProgram(program);
    glBindVertexArray(geometry.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
    glDrawElements(GL_TRIANGLES, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
  }

  inline const glm::mat4& projection() const {
    return *proj_matrix;
  }

  inline const glm::mat4& view() const {
    return *view_matrix;
  }

  void setClearColor(const glm::vec4& clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
  }

  void enableDepthBuffer() {
    glEnable(GL_DEPTH_TEST);
  }

  void disableDepthBuffer() {
    glDisable(GL_DEPTH_TEST);
  }

  void setLightPos(GLuint light, const glm::vec4& pos) {
    lights[light].pos = pos;
  }

  void setLightDiffuse(GLuint light, glm::vec4& diffuse) {
    lights[light].diffuse = diffuse;
  }

  void setLightSpecular(GLuint light, glm::vec4& specular) {
    lights[light].specular = specular;
  }

  void setLight(GLuint l, Light& light) {
    lights[l] = light;
  }

  void disableLight(GLuint light) {
    // TODO: Implement enabling/disabling of lights
  }

  void enableLight(GLuint light) {
    // TODO: Implement enabling disabling of lights
  }

  void setViewMatrix(const glm::mat4& matrix) {
    *view_matrix = matrix;
  }

  void setViewLookat(const glm::vec3& eye, const glm::vec3& ctr, const glm::vec3& up) {
    *view_matrix = glm::lookAt(eye, ctr, up);
  }

  void setPerspectiveProjection(GLfloat fov_y, GLfloat aspect, GLfloat near_z, GLfloat far_z) {
    *proj_matrix = glm::perspective(fov_y, aspect, near_z, far_z);
  }

  void setPerspectiveProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    *proj_matrix = glm::frustum(left, right, bottom, top, near, far);
  }

  void setOrthographicProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    *proj_matrix = glm::ortho(left, right, bottom, top, near, far);
  }
};

#endif /* ENGINE_H_ */
