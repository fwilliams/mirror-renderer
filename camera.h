#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
  glm::quat orientation;
  glm::vec3 lookat = glm::vec3(0.0, 0.0, 1.0);
  glm::vec3 position;
  glm::mat4 proj_mat;
public:
  Camera();
  virtual ~Camera();

  void rotateX(float angle);
  void rotateY(float angle);
  void rotateZ(float angle);

  void transform(const glm::mat4& transformation);
  void advance(float distance);
  void strafeRight(float amount);
  void strafeUp(float amount);

  void setPosition(const glm::vec3& position);
  void setOrientation(const glm::vec3& direction);
  void setPerspectiveProjection(float left, float right, float bottom, float top, float near, float far);
  void setPerspectiveProjection(float fov_y, float aspect, float near_z, float far_z);
  void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);

  glm::mat4 getViewMatrix() const;
  glm::mat4 getProjectionMatrix() const;
  glm::vec3 getLookatVector() const;
  glm::vec3 getUpVector() const;
  glm::vec3 getRightVector() const;
  glm::vec3 getPosition() const;
};

#endif /* CAMERA_H_ */
