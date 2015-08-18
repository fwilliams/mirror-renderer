#include <GL/glew.h>

#include <cassert>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#ifndef RENDERER_MATERIAL_H_
#define RENDERER_MATERIAL_H_

class Renderer;

class Material {
	friend class Renderer;

	glm::vec3 m_diffuse = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 m_specular = glm::vec3(0.0, 0.0, 0.0);
	float m_roughness = 0.0;
	float m_reflectance = 0.0;

	unsigned* m_program = 0;

	Material() = default;

	Material(const glm::vec3& d, const glm::vec3& s, float m, float r) :
		m_diffuse(d), m_specular(s), m_roughness(m), m_reflectance(r) {}

	void setupUniforms() {
		static GLuint MAT_LOC_DIFF = glGetUniformLocation(*m_program, "mat.diffuse");
		static GLuint MAT_LOC_SPEC = glGetUniformLocation(*m_program, "mat.specular");
		static GLuint MAT_LOC_SHINE = glGetUniformLocation(*m_program, "mat.roughness");
		static GLuint MAT_LOC_REFL = glGetUniformLocation(*m_program, "mat.reflectance");

		glUniform4fv(MAT_LOC_DIFF, 1, glm::value_ptr(m_diffuse));
		glUniform4fv(MAT_LOC_SPEC, 1, glm::value_ptr(m_specular));
		glUniform1f(MAT_LOC_SHINE, shininess());
		glUniform1f(MAT_LOC_REFL, m_reflectance);
	}

public:
	glm::vec3 diffuse() const {
		return m_diffuse;
	}

	glm::vec3 specular() const {
		return m_specular;
	}

	float roughness() const {
		return m_roughness;
	}

	float shininess() const {
		return 2.0f + (2.0f / (m_roughness * m_roughness));
	}

	float reflectance() const {
		return m_reflectance;
	}

	void setDiffuse(const glm::vec3& diff) {
		m_diffuse = diff;
	}

	void setSpecular(const glm::vec3& spec) {
		m_specular = spec;
	}

	void setRoughness(float m) {
		assert(m < 1.0 && m > 0.0);
		m_roughness = m;
	}

	void setReflectance(float r) {
		assert(r < 1.0 && r > 0.0);
		m_reflectance = r;
	}

	void setShininess(float s) {
		m_roughness = sqrt(2.0f / (s + 2.0f));
	}
};



#endif /* RENDERER_MATERIAL_H_ */
