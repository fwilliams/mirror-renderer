#include "camera.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

Camera::Camera() {
  // TODO Auto-generated constructor stub

}

Camera::~Camera() {
  // TODO Auto-generated destructor stub
}

void Camera::rotateX(float angle) {
  orientation = orientation * quat(vec3(angle, 0.0, 0.0));
}

void Camera::rotateY(float angle) {
  orientation = orientation * quat(vec3(0.0, angle, 0.0));
}

void Camera::rotateZ(float angle) {
  orientation = orientation * quat(vec3(0.0, 0.0, angle));
}

void Camera::advance(float amount) {
  vec3 dir = getLookatVector() * amount;
  position += dir;
}

void Camera::setOrientation(const vec3& lookat) {
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
  vec3 lookat = mat3_cast(orientation) * this->lookat;
  return lookAt(position, position + lookat, vec3(0.0, 1.0, 0.0));
}

mat4 Camera::getProjectionMatrix() const {
  return proj_mat;
}

vec3 Camera::getLookatVector() const {
  return mat3_cast(orientation) * this->lookat;
}

vec3 Camera::getPosition() const {
  return position;
}

