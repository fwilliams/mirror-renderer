#include <GL/glew.h>

#include <memory>

#include <glm/glm.hpp>

#include "geometry/3d_primitives.h"
#include "utils/gl_program_builder.h"
#include "material.h"

#ifndef RENDERER_RENDERER_H_
#define RENDERER_RENDERER_H_

struct Light {
  glm::vec4 pos;
  glm::vec4 color;
  glm::float32 attenuation;
  glm::float32 enabled;
  glm::float32 dummy1;
  glm::float32 dummy2;
  glm::float32 dummy3;
};

enum PrimitiveType { TRIANGLES = GL_TRIANGLES, LINES = GL_LINES, POINTS = GL_POINTS };
enum FaceCullMode { BACK = GL_BACK, FRONT = GL_FRONT, FRONT_AND_BACK = GL_FRONT_AND_BACK };
enum WindingMode { CW, CCW };

class Renderer {
	static const GLuint NUM_LIGHTS = 10;

	struct PerFrameData {
		glm::mat4 modelview_matrix;
		glm::mat4 normal_matrix;
		glm::mat4 view_matrix;
		glm::mat4 proj_matrix;
		glm::vec4 global_ambient;
		Light lights[NUM_LIGHTS];
	};

	constexpr static const char* MV_MAT_UNIFORM_NAME = "std_Modelview";
	constexpr static const char* NORMAL_MAT_UNIFORM_NAME = "std_Normal";
	constexpr static const char* VIEW_MAT_UNIFORM_NAME = "std_View";
	constexpr static const char* PROJ_MAT_UNIFORM_NAME = "std_Projection";
	constexpr static const char* GLOB_AMB_UNIFORM_NAME = "std_GlobalAmbient";
	constexpr static const char* LIGHTS_UNIFORM_NAME = "std_Lights";

	PerFrameData perFrameData;

	GLuint currentProgram = 0;

	utils::GLProgramBuilder programBuilder;

	void setupUniforms();

	GLuint materialProgram;
	GLuint drawLightsProgram;
	GLuint drawNormalsProgram;

public:

	Renderer();

	~Renderer();

	template <typename... Args>
	std::shared_ptr<Material> createMaterial(Args... args) {
		auto r = std::make_shared<Material>(Material(std::forward<Args>(args)...));
		r->m_program = &materialProgram;
		return r;
	}

	void startFrame();

	void draw(GLuint vao, size_t num_vertices, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

	void draw(const Geometry& geometry, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

	void drawLights(const Geometry& g, const glm::mat4& transform, const PrimitiveType& p = TRIANGLES);

	void drawNormals(const Geometry& g, const glm::mat4& transform, const glm::vec4& color1, const glm::vec4& color2);

	void setProgram(GLuint program);

	void setMaterial(std::shared_ptr<Material> mat) {
		setProgram(*mat->m_program);
		mat->setupUniforms();
	}

	void clearViewport() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	inline const glm::mat4& projectionMatrix() const {
		return perFrameData.proj_matrix;
	}

	inline const glm::mat4& viewMatrix() const {
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

	void setGlobalAmbient(const glm::vec4& globalAmb) {
		perFrameData.global_ambient = globalAmb;
	}

	void setLightAttenuation(GLuint light, GLfloat attenuation) {
		perFrameData.lights[light].attenuation = 1.0f / glm::pow(attenuation, 2.0);
	}

	void setLightPos(GLuint light, const glm::vec4& pos) {
		perFrameData.lights[light].pos = pos;
	}

	void setLightColor(GLuint light, const glm::vec4& color) {
		perFrameData.lights[light].color = color;
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

	void enableAlphaBlending() {
		glEnable(GL_BLEND);
	}

  void disableAlphaBlending() {
    glDisable(GL_BLEND);
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
};

#endif /* RENDERER_RENDERER_H_ */
