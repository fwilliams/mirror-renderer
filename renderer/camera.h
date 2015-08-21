#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
  glm::quat orientation = glm::quat(1.0, glm::vec3(0.0, 0.0, 0.0));
  glm::vec3 lookat = glm::vec3(0.0, 0.0, 1.0);
  glm::vec3 position = glm::vec3(0.0, 0.0, 0.0);
  glm::mat4 proj_mat;
  glm::vec2 fov;
  glm::mat4 viewTransform = glm::mat4(1.0);

public:
  void rotateX(float angle);
  void rotateY(float angle);
  void rotateZ(float angle);

  void setRotationAngles(const glm::vec3& angles);

  void transformView(const glm::mat4& transformation);
  void advance(float distance);
  void strafeRight(float amount);
  void strafeUp(float amount);

  void setPosition(const glm::vec3& position);
  void setLookat(const glm::vec3& direction);
  void setPerspectiveProjection(float left, float right, float bottom, float top, float near, float far);
  void setPerspectiveProjection(float fov_y, float aspect, float near_z, float far_z);
  void setOrthographicProjection(float left, float right, float bottom, float top, float near, float far);

  glm::mat4 getViewMatrix() const;
  glm::mat4 getProjectionMatrix() const;
  glm::vec3 getLookatVector() const;
  glm::vec3 getUpVector() const;
  glm::vec3 getRightVector() const;
  glm::vec3 getPosition() const;
  float getFovY() const;
  float getFovX() const;
  float getFovYRadians() const;
  float getFovXRadians() const;
};

class FirstPersonCamera : public Camera {
  glm::vec3 moveCamera = glm::vec3(0, 0, 0);
  glm::vec2 cameraSphericalCoords;
  glm::vec3 cameraVelocity = glm::vec3(1.0);
  glm::vec2 cameraAngularVel = glm::vec2(1.0);

public:
  FirstPersonCamera() = default;
  FirstPersonCamera(const glm::vec3& vel, const glm::vec2& angVel);

  enum class CameraDirection { POSITIVE, NEGATIVE, STOPPED };
  void setHorizontalDirection(const CameraDirection& dir);
  void setForwardDirection(const CameraDirection& dir);
  void setUpDirection(const CameraDirection& dir);
  void setDirection(const CameraDirection& dirH, const CameraDirection& dirF);
  void setCameraVelocity(const glm::vec3& vel);
  void setAngularVelocity(const glm::vec2& vel);

  void updateLookat(const glm::vec2& pos);
  void updatePosition();

  glm::vec3 getCameraVelocity() const {
    return moveCamera * cameraVelocity;
  }
};
#endif /* CAMERA_H_ */
