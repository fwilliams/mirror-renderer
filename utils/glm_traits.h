#include <glm/glm.hpp>

#include <GL/glew.h>

#ifndef UTILS_GLM_TRAITS_H_
#define UTILS_GLM_TRAITS_H_

template <typename T>
constexpr GLuint dim();

template <>
constexpr GLuint dim<glm::vec4>() { return 4; }

template <>
constexpr GLuint dim<glm::vec3>() { return 3; }

template <>
constexpr GLuint dim<glm::vec2>() { return 2; }

template <>
constexpr GLuint dim<glm::float_t>() { return 1; }

template <>
constexpr GLuint dim<glm::ivec4>() { return 4; }

template <>
constexpr GLuint dim<glm::ivec3>() { return 3; }

template <>
constexpr GLuint dim<glm::ivec2>() { return 2; }

template <>
constexpr GLuint dim<glm::int_t>() { return 1; }

template <>
constexpr GLuint dim<glm::uvec4>() { return 4; }

template <>
constexpr GLuint dim<glm::uvec3>() { return 3; }

template <>
constexpr GLuint dim<glm::uvec2>() { return 2; }

template <>
constexpr GLuint dim<glm::uint_t>() { return 1; }



template <typename T>
constexpr GLenum gl_type_id() {
  return gl_type_id<typename T::value_type>();
}

template <>
constexpr GLenum gl_type_id<glm::float32>() { return GL_FLOAT; }

template <>
constexpr GLenum gl_type_id<glm::float64>() { return GL_DOUBLE; }

template <>
constexpr GLenum gl_type_id<glm::uint32>() { return GL_UNSIGNED_INT; }

template <>
constexpr GLenum gl_type_id<glm::uint16>() { return GL_UNSIGNED_SHORT; }

template <>
constexpr GLenum gl_type_id<glm::uint8>() { return GL_UNSIGNED_BYTE; }

template <>
constexpr GLenum gl_type_id<glm::int32>() { return GL_UNSIGNED_INT; }

template <>
constexpr GLenum gl_type_id<glm::int16>() { return GL_UNSIGNED_SHORT; }

template <>
constexpr GLenum gl_type_id<glm::int8>() { return GL_UNSIGNED_BYTE; }

#endif /* UTILS_GLM_TRAITS_H_ */
