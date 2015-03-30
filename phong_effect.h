#include <glm/glm.hpp>
#include <GL/glew.h>

#ifndef PHONGEFFECT_H_
#define PHONGEFFECT_H_

class PhongEffect {
  GLuint program_id;

  const GLuint MV_MAT_LOC = 0;
  const GLuint PROJ_MAT_LOC = 1;
  const GLuint NORMAL_MAT_LOC = 2;
  const GLuint MATERIAL_LOC = 3;
  const GLuint AMBIENT_LOC = 6;
  const GLuint LIGHTS_LOC = 7;
  const GLuint COLOR1_LOC = 2;
  const GLuint COLOR2_LOC = 3;

public:
  PhongEffect();
  virtual ~PhongEffect();

  void setModelViewMat(glm::mat4& modelview);
  void setProjectionMat(glm::mat4& proj);
  void setGlobalAmbient(glm::vec4& color);
  void setMaterialDiffuse(glm::vec4& diffuse);
  void setMaterialSpecular(glm::vec4& specular);
  void setMaterialShineExp(GLfloat shine);

  void draw();
};

#endif /* PHONGEFFECT_H_ */
