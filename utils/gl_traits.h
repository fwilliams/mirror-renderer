#include <glm/glm.hpp>

#include <GL/glew.h>

#ifndef UTILS_GLM_TRAITS_H_
#define UTILS_GLM_TRAITS_H_

namespace utils {
/*
 * Determines whether a type is a valid GLSL type
 * Note that other than GLint, GLuint, and GLfloat,
 * only glm::T types are accepted
 * TODO: double vectors and matrices
 */
template <typename T>
constexpr bool is_glsl_type() {
	return false;
}

template <>
constexpr bool is_glsl_type<GLboolean>() {
	return true;
}

template <>
constexpr bool is_glsl_type<GLuint>() {
	return true;
}

template <>
constexpr bool is_glsl_type<GLint>() {
	return true;
}

template <>
constexpr bool is_glsl_type<GLfloat>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::vec2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::vec3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::vec4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::ivec2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::ivec3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::ivec4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::uvec2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::uvec3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::uvec4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::bvec2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::bvec3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::bvec4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat2x3>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat3x2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat2x4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat4x2>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat3x4>() {
	return true;
}

template <>
constexpr bool is_glsl_type<glm::mat4x3>() {
	return true;
}


/*
 * Gets the number of dimensions for a type.
 * E.g. vec3 has 3 dimensions
 * TODO: Missing a lot of types
 */
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



/*
 * Gets the OpenGL type constant for the value type contained in a given type
 * E.g. The value type of vec2 is float, so the value type id for vec2 would be GL_FLOAT.
 * Note that other than GL(u)int, GL(u)short, GL(u)byte, GLfloat, and GLdouble
 * only glm::T types are accepted
 */
template <typename T>
constexpr GLenum gl_value_type_id() {
	static_assert(is_glsl_type<T>(), "Error: type given to gl_value_type_id is not a valid GLSL type.");
	return gl_value_type_id<typename T::value_type>();
}

template <>
constexpr GLenum gl_value_type_id<GLfloat>() { return GL_FLOAT; }

template <>
constexpr GLenum gl_value_type_id<GLdouble>() { return GL_DOUBLE; }

template <>
constexpr GLenum gl_value_type_id<GLint>() { return GL_INT; }

template <>
constexpr GLenum gl_value_type_id<GLuint>() { return GL_UNSIGNED_INT; }

template <>
constexpr GLenum gl_value_type_id<GLshort>() { return GL_SHORT; }

template <>
constexpr GLenum gl_value_type_id<GLushort>() { return GL_UNSIGNED_SHORT; }

template <>
constexpr GLenum gl_value_type_id<GLbyte>() { return GL_BYTE; }

template <>
constexpr GLenum gl_value_type_id<GLubyte>() { return GL_UNSIGNED_BYTE; }


/*
 * Gets the OpenGL type constant for a given type.
 * E.g. ivec has the type constant GL_INT_VEC2
 * TODO: handle double vectors and matrices
 */
template <class T>
constexpr GLenum gl_type_id() {
	static_assert(is_glsl_type<T>(), "Error: type given to gl_type_id is not a valid GLSL type.");
	return 0;
}

template <>
constexpr GLenum gl_type_id<GLfloat>() { return GL_FLOAT; }

template <>
constexpr GLenum gl_type_id<GLdouble>() { return GL_DOUBLE; }

template <>
constexpr GLenum gl_type_id<GLint>() { return GL_INT; }

template <>
constexpr GLenum gl_type_id<GLuint>() { return GL_UNSIGNED_INT; }

template <>
constexpr GLenum gl_type_id<GLshort>() { return GL_SHORT; }

template <>
constexpr GLenum gl_type_id<GLushort>() { return GL_UNSIGNED_SHORT; }

template <>
constexpr GLenum gl_type_id<GLbyte>() { return GL_BYTE; }

template <>
constexpr GLenum gl_type_id<GLubyte>() { return GL_UNSIGNED_BYTE; }

template <>
constexpr GLenum gl_type_id<glm::vec2>() { return GL_FLOAT_VEC2; }

template <>
constexpr GLenum gl_type_id<glm::vec3>() { return GL_FLOAT_VEC3; }

template <>
constexpr GLenum gl_type_id<glm::vec4>() { return GL_FLOAT_VEC4; }

template <>
constexpr GLenum gl_type_id<glm::ivec2>() { return GL_INT_VEC2; }

template <>
constexpr GLenum gl_type_id<glm::ivec3>() { return GL_INT_VEC3; }

template <>
constexpr GLenum gl_type_id<glm::ivec4>() { return GL_INT_VEC4; }

template <>
constexpr GLenum gl_type_id<glm::uvec2>() { return GL_UNSIGNED_INT_VEC2; }

template <>
constexpr GLenum gl_type_id<glm::uvec3>() { return GL_UNSIGNED_INT_VEC3; }

template <>
constexpr GLenum gl_type_id<glm::uvec4>() { return GL_UNSIGNED_INT_VEC4; }

//template <>
//constexpr GLenum gl_type_id<glm::bvec2>() { return GL_BOOL_VEC2; }
//
//template <>
//constexpr GLenum gl_type_id<glm::bvec3>() { return GL_BOOL_VEC3; }
//
//template <>
//constexpr GLenum gl_type_id<glm::bvec4>() { return GL_BOOL_VEC4; }

template <>
constexpr GLenum gl_type_id<glm::mat2>() { return GL_FLOAT_MAT2; }

template <>
constexpr GLenum gl_type_id<glm::mat3>() { return GL_FLOAT_MAT3; }

template <>
constexpr GLenum gl_type_id<glm::mat4>() { return GL_FLOAT_MAT4; }

template <>
constexpr GLenum gl_type_id<glm::mat2x3>() { return GL_FLOAT_MAT2x3; }

template <>
constexpr GLenum gl_type_id<glm::mat3x2>() { return GL_FLOAT_MAT3x2; }

template <>
constexpr GLenum gl_type_id<glm::mat2x4>() { return GL_FLOAT_MAT2x4; }

template <>
constexpr GLenum gl_type_id<glm::mat4x2>() { return GL_FLOAT_MAT4x2; }

template <>
constexpr GLenum gl_type_id<glm::mat4x3>() { return GL_FLOAT_MAT4x3; }

template <>
constexpr GLenum gl_type_id<glm::mat3x4>() { return GL_FLOAT_MAT3x4; }




/*
 * Gets the type of a single unit in a container
 * E.g. vec2 contains floats, ivec2 contains ints so their
 *      container types will be GLfloat, and GLint respectively
 * TODO: double vectors and matrices
 */
template <class T>
struct container_type;

template <>
struct container_type<GLfloat> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::vec2> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::vec3> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::vec4> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat2> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat3> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat4> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat2x3> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat3x2> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat2x4> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat4x2> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat3x4> {
	typedef GLfloat type;
};

template <>
struct container_type<glm::mat4x3> {
	typedef GLfloat type;
};

template <>
struct container_type<GLint> {
	typedef GLint type;
};

template <>
struct container_type<glm::ivec2> {
	typedef GLint type;
};

template <>
struct container_type<glm::ivec3> {
	typedef GLint type;
};

template <>
struct container_type<glm::ivec4> {
	typedef GLint type;
};

template <>
struct container_type<GLuint> {
	typedef GLuint type;
};

template <>
struct container_type<glm::uvec2> {
	typedef GLuint type;
};

template <>
struct container_type<glm::uvec3> {
	typedef GLuint type;
};

template <>
struct container_type<glm::uvec4> {
	typedef GLuint type;
};

template <>
struct container_type<GLboolean> {
	typedef GLboolean type;
};

template <>
struct container_type<glm::bvec2> {
	typedef GLboolean type;
};

template <>
struct container_type<glm::bvec3> {
	typedef GLboolean type;
};

template <>
struct container_type<glm::bvec4> {
	typedef GLboolean type;
};
}

#endif /* UTILS_GLM_TRAITS_H_ */
