#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/geometry.h"

#ifndef ENGINE_H_
#define ENGINE_H_

struct Light {
  glm::vec4 pos = glm::vec4(0.0, 0.0, 0.0, 1.0);
  glm::vec4 diffuse = glm::vec4(0.0, 0.0, 0.0, 1.0);
  glm::vec4 specular = glm::vec4(0.0, 0.0, 0.0, 1.0);
};

class Renderer {
  GLuint matrices_ubo = 0;
  GLuint lights_ubo = 0;
  GLuint draw_normals_program = 0;

  const GLuint MATS_UBO_BINDING_POINT = 1;
  const GLuint LIGHTS_UBO_BINDING_POINT = 2;

  const GLuint COLOR1_LOC = 2;
  const GLuint COLOR2_LOC = 3;

  const GLuint NUM_LIGHTS = 10;

  Light* lights;
  glm::mat4* matrices;

  inline glm::mat4* projection() {
    return &matrices[0];
  }

  inline glm::mat4* view() {
    return &matrices[1];
  }

  inline glm::mat4* normal() {
    return &matrices[2];
  }

public:
  Renderer();
  virtual ~Renderer();

  void drawNormals(glm::vec4 baseColor, glm::vec4 tailColor, Geometry& geometry) {
    glBindVertexArray(geometry.normal_view_vao);
    glUseProgram(draw_normals_program);
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
    glUseProgram(program);
    glBindVertexArray(geometry.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry.ibo);
    glDrawElements(GL_TRIANGLES, geometry.num_indices, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);
  }

  void setClearColor(glm::vec4& clearColor) {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
  }

  void enableDepthBuffer() {
    glEnable(GL_DEPTH_TEST);
  }

  void disableDepthBuffer() {
    glDisable(GL_DEPTH_TEST);
  }

  void setLightPos(GLuint light, glm::vec4& pos) {
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

  void setViewMatrix(glm::mat4& matrix) {
    *view() = matrix;
  }

  void setPerspectiveProjection(GLfloat fov_y, GLfloat aspect, GLfloat near_z, GLfloat far_z) {
    *projection() = glm::perspective(fov_y, aspect, near_z, far_z);
  }

  void setPerspectiveProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    *projection() = glm::frustum(left, right, bottom, top, near, far);
  }

  void setOrthographicProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    *projection() = glm::ortho(left, right, bottom, top, near, far);
  }
};

#endif /* ENGINE_H_ */
