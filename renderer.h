#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "utils/geometry.h"

#ifndef RENDERER_H_
#define RENDERER_H_

struct Light {
  glm::vec4 pos;
  glm::vec4 diffuse;
  glm::vec4 specular;
};

class Renderer {
  static const GLuint MATS_UBO_BINDING_POINT = 1;
  static const GLuint LIGHTS_UBO_BINDING_POINT = 2;

  static const GLuint COLOR1_LOC = 2;
  static const GLuint COLOR2_LOC = 3;

  static const GLuint NUM_LIGHTS = 10;

  struct PerFrameData {
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::mat4 normal_matrix;
    glm::mat4 dummy; // FIXME: Uniform bindings need to be 256 byte aligned
    glm::vec4 global_ambient;
    Light lights[NUM_LIGHTS];
  };

  GLuint per_frame_ubo = 0;
  GLuint draw_normals_program = 0;

  PerFrameData perFrameData;

public:
  Renderer();
  virtual ~Renderer();

  void startFrame();

  void drawNormals(glm::vec4 baseColor, glm::vec4 tailColor, Geometry& geometry);

  void drawWireframe(Geometry& geometry);

  void draw(Geometry& geometry);

  void setProgram(GLuint program);

  void clearViewPort() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  inline const glm::mat4& projection() const {
    return perFrameData.proj_matrix;
  }

  inline const glm::mat4& view() const {
    return perFrameData.view_matrix;
  }

  inline const glm::vec4& globalAmbient() const {
    return perFrameData.global_ambient;
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

  void setGlobalAmbient(const glm::vec4& globalAmb) {
    perFrameData.global_ambient = globalAmb;
  }

  void setLightPos(GLuint light, const glm::vec4& pos) {
    perFrameData.lights[light].pos = pos;
  }

  void setLightDiffuse(GLuint light, const glm::vec4& diffuse) {
    perFrameData.lights[light].diffuse = diffuse;
  }

  void setLightSpecular(GLuint light, const glm::vec4& specular) {
    perFrameData.lights[light].specular = specular;
  }

  void setLight(GLuint l, const Light& light) {
    perFrameData.lights[l] = light;
  }

  void disableLight(GLuint light) {
    // TODO: Implement enabling/disabling of lights
  }

  void enableLight(GLuint light) {
    // TODO: Implement enabling disabling of lights
  }

  void setViewMatrix(const glm::mat4& matrix) {
    perFrameData.view_matrix = matrix;
  }

  void setViewLookat(const glm::vec3& eye, const glm::vec3& ctr, const glm::vec3& up) {
    perFrameData.view_matrix = glm::lookAt(eye, ctr, up);
  }

  void setProjectionMatrix(const glm::mat4& matrix) {
    perFrameData.proj_matrix = matrix;
  }

  void setPerspectiveProjection(GLfloat fov_y, GLfloat aspect, GLfloat near_z, GLfloat far_z) {
    perFrameData.proj_matrix = glm::perspective(fov_y, aspect, near_z, far_z);
  }

  void setPerspectiveProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    perFrameData.proj_matrix = glm::frustum(left, right, bottom, top, near, far);
  }

  void setOrthographicProjection(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat near, GLfloat far) {
    perFrameData.proj_matrix = glm::ortho(left, right, bottom, top, near, far);
  }
};

#endif /* RENDERER_H_ */
