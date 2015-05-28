#include <GL/glew.h>

#include <unordered_map>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "geometry/3d_primitives.h"
#include "gl_program_builder.h"

#ifndef RENDERER_H_
#define RENDERER_H_

namespace renderer {

struct Light {
  glm::vec4 pos;
  glm::vec4 diffuse;
  glm::vec4 specular;
  glm::float32 attenuation;
  glm::float32 enabled;
  glm::float32 dummy1;
  glm::float32 dummy2;
};

enum class GlVersion {
  GL330, GL430
};

template <GlVersion V>
inline std::string gl_version_to_string();

template <>
inline std::string gl_version_to_string<GlVersion::GL330>() { return "330"; };

template <>
inline std::string gl_version_to_string<GlVersion::GL430>() { return "430"; };

enum FaceCullMode { BACK = GL_BACK, FRONT = GL_FRONT, FRONT_AND_BACK = GL_FRONT_AND_BACK };
enum PrimitiveType { TRIANGLES = GL_TRIANGLES, LINES = GL_LINES, POINTS = GL_POINTS };
enum WindingMode { CW, CCW };

template <GlVersion V>
class Renderer {
  static const GLuint MATS_UBO_BINDING_POINT = 1;
  static const GLuint LIGHTS_UBO_BINDING_POINT = 2;

  static const GLuint NUM_LIGHTS = 10;

  constexpr static const char* MV_MAT_UNIFORM_NAME = "std_Modelview";
  constexpr static const char* NORMAL_MAT_UNIFORM_NAME = "std_Normal";
  constexpr static const char* VIEW_MAT_UNIFORM_NAME = "std_View";
  constexpr static const char* PROJ_MAT_UNIFORM_NAME = "std_Projection";
  constexpr static const char* GLOB_AMB_UNIFORM_NAME = "std_GlobalAmbient";
  constexpr static const char* LIGHTS_UNIFORM_NAME = "std_Lights";

  struct PerFrameData {
    glm::mat4 modelview_matrix;
    glm::mat4 normal_matrix;
    glm::mat4 view_matrix;
    glm::mat4 proj_matrix;
    glm::vec4 global_ambient;
    Light lights[NUM_LIGHTS];
  };

  GLuint perFrameUbo = 0;

  GLuint currentProgram = 0;

  PerFrameData perFrameData;

  detail::GLProgramBuilder<V> programBuilder;

  void setupUniforms();

public:

  Renderer();
  virtual ~Renderer();

  void startFrame();

  void draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

  void draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

  void setProgram(GLuint program);

  GLuint makeProgramFromFiles(const std::string& vert, const std::string& frag) {
    return programBuilder.buildFromFiles(vert, frag);
  }

  GLuint makeProgramFromStrings(const std::string& vert, const std::string& frag) {
    return programBuilder.buildFromStrings(vert, frag);
  }

  GLuint makeComputeProgramFromFile(const std::string& shader);

  GLuint makeComputeProgramFromString(const std::string& shader);

  void addShaderIncludeDir(const std::string& dir) {
    programBuilder.addIncludeDir(dir);
  }

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

  void enableAlphaBlending() {
    glEnable(GL_BLEND);
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
}

#endif /* RENDERER_H_ */
