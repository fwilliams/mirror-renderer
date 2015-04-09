#include "camera.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

void Camera::rotateX(float angle) {
  orientation = rotate(orientation, angle, vec3(1.0, 0.0, 0.0));
}

void Camera::rotateY(float angle) {
  orientation = rotate(orientation, angle, vec3(0.0, 1.0, 0.0));
}

void Camera::rotateZ(float angle) {
  orientation = rotate(orientation, angle, vec3(0.0, 0.0, 1.0));
}

void Camera::advance(float amount) {
  position += getLookatVector() * amount;
}

void Camera::strafeRight(float amount) {
  position += getRightVector() * amount;
}

void Camera::strafeUp(float amount) {
  position += getUpVector() * amount;
}

void Camera::setLookat(const vec3& lookat) {
  orientation = glm::quat(1.0, glm::vec3(0.0));
  this->lookat = lookat;
}

void Camera::setPosition(const vec3& position) {
  this->position = position;
}

void Camera::setPerspectiveProjection(float left, float right, float bottom, float top,
    float near, float far) {
  proj_mat = frustum(left, right, bottom, top, near, far);
}

void Camera::setOrthographicProjection(float left, float right, float bottom, float top,
    float near, float far) {
  proj_mat = ortho(left, right, bottom, top, near, far);
}

void Camera::setPerspectiveProjection(float fov_y, float aspect, float near_z,
    float far_z) {
  proj_mat = perspective(fov_y, aspect, near_z, far_z);
}

mat4 Camera::getViewMatrix() const {
  return lookAt(getPosition(), getPosition() + getLookatVector(), getUpVector());
}

mat4 Camera::getProjectionMatrix() const {
  return proj_mat;
}

vec3 Camera::getLookatVector() const {
  return orientation * lookat;
}

vec3 Camera::getUpVector() const {
  return orientation * vec3(0.0, 1.0, 0.0);
}

vec3 Camera::getRightVector() const {
  return normalize(cross(getLookatVector(), getUpVector()));
}

vec3 Camera::getPosition() const {
  return position;
}

