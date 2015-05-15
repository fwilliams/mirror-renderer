#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geometry/3d_primitives.h"

#ifndef RENDERER_H_
#define RENDERER_H_

struct Light {
  glm::vec4 pos;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::float32 attenuation;
  glm::float32 enabled;
  glm::float32 dummy1;
  glm::float32 dummy2;
};

class Renderer {
  static const GLuint MATS_UBO_BINDING_POINT = 1;
  static const GLuint LIGHTS_UBO_BINDING_POINT = 2;

  static const GLuint NUM_LIGHTS = 10;

  struct PerFrameData {
    glm::mat4 modelview_matrix;
    glm::mat4 normal_matrix;
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::vec4 global_ambient;
    Light lights[NUM_LIGHTS];
  };

  GLuint per_frame_ubo = 0;

  PerFrameData perFrameData;

public:
  enum FaceCullMode { BACK = GL_BACK, FRONT = GL_FRONT, FRONT_AND_BACK = GL_FRONT_AND_BACK };
  enum PrimitiveType { TRIANGLES = GL_TRIANGLES, LINES = GL_LINES, POINTS = GL_POINTS };
  enum WindingMode { CW, CCW };

  Renderer();
  virtual ~Renderer();

  void startFrame();

  void draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

  void draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

  void setProgram(GLuint program);

  void clearViewport() {
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

  inline size_t numLights() const {
    return NUM_LIGHTS;
  }

  glm::vec3 lightPosition(size_t i) {
    return glm::vec3(perFrameData.lights[i].pos);
  }

  void enableFaceCulling() {
    glEnable(GL_CULL_FACE);
  }

  void disableFaceCulling() {
    glDisable(GL_CULL_FACE);
  }

  void setFaceCullMode(FaceCullMode mode) {
    glCullFace(mode);
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

  void setLightAttenuation(GLuint light, GLfloat attenuation) {
    perFrameData.lights[light].attenuation = 1.0f / glm::pow(attenuation, 2.0);
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
    perFrameData.lights[light].enabled = 0.0; //glm::bvec1(false);
  }

  void enableLight(GLuint light) {
    perFrameData.lights[light].enabled = 1.0; //glm::bvec1(true);
  }

  void setViewMatrix(const glm::mat4& matrix) {
    perFrameData.view_matrix = matrix;
  }

  void setProjectionMatrix(const glm::mat4& matrix) {
    perFrameData.proj_matrix = matrix;
  }
};

#endif /* RENDERER_H_ */