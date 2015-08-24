#include <GL/glew.h>

#include "tuple.h"

#ifndef GFX_UTILS_VERTEX_H_
#define GFX_UTILS_VERTEX_H_

namespace geometry {

namespace detail {
/*
 * Setup vertex attribute pointers for a cons-list of types
 */
template <size_t OFF, size_t I, class ConsCell>
struct EnableAttribArrayAOS;

template <size_t OFF, size_t I>
struct EnableAttribArrayAOS<OFF, I, EmptyListType> {
  inline static void enable(size_t stride) {
    return;
  }
};

template <size_t OFF, size_t I, class ConsCell>
struct EnableAttribArrayAOS {
  inline static void enable(size_t stride) {
    typedef typename ConsCell::HeadType HT;
    typedef typename ConsCell::TailType TT;
    typedef ListElement<0, ConsCell> LE;

//    std::cout << "glEnableVertexAttribArray(" << I << ");" << std::endl;
//    std::cout << "glVertexAttribPointer(" << I << ", " << utils::dim<HT>() << ", " << utils::gl_value_type_id<HT>() << ", " << GL_FALSE << ", " << stride << ", " << LE::offset() + OFF << ");" << std::endl;
    glEnableVertexAttribArray(I);
    glVertexAttribPointer(I, utils::dim<HT>(), utils::gl_value_type_id<HT>(), GL_FALSE, stride, (void*)(LE::offset() + OFF));

    EnableAttribArrayAOS<LE::offset() + OFF + sizeof(HT), I+1, TT>::enable(stride);
  }
};

}

/*
 * Generate a VAO for an array of vertices of type Vertex
 * Vertex must be have the trait IsTypeTuple
 * TODO: Test with program which doesn't use middle attributes
 */
template <class Vertex>
inline static GLuint generateVAO() {
  static_assert(IsGfxTuple<Vertex>::value,
      "Error Vertex type is not a TupleN or TypeList.");
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  const size_t sizeofVertex = sizeof(Vertex);

  detail::EnableAttribArrayAOS<0, 0, typename Vertex::ListType>::enable(sizeofVertex);
  return vao;
}


struct Vertex4P3N2T : public Tuple<glm::vec4, glm::vec3, glm::vec2> {
	enum { POS_I = 0, NORM_I = 1, TEX_I = 2};

	inline glm::vec4& position() {
		return get<POS_I>();
	}

	inline glm::vec3& normal() {
		return get<NORM_I>();
	}

	inline glm::vec2& texcoord() {
		return get<TEX_I>();
	}

	inline const glm::vec4& position() const {
		return get<POS_I>();
	}

	inline const glm::vec3& normal() const {
		return get<NORM_I>();
	}

	inline const glm::vec2& texcoord() const {
		return get<TEX_I>();
	}
};

struct Vertex4P3N : public Tuple<glm::vec4, glm::vec3> {
	enum { POS_I = 0, NORM_I = 1};

	inline glm::vec4& position() {
		return get<POS_I>();
	}

	inline glm::vec3& normal() {
		return get<NORM_I>();
	}

	inline const glm::vec4& position() const {
		return get<POS_I>();
	}

	inline const glm::vec3& normal() const {
		return get<NORM_I>();
	}
};

struct Vertex4P2T : public Tuple<glm::vec4, glm::vec2> {
	enum { POS_I = 0, TEX_I = 1};

	inline glm::vec4& position() {
		return get<POS_I>();
	}

	inline glm::vec2& texcoord() {
		return get<TEX_I>();
	}

	inline const glm::vec4& position() const {
		return get<POS_I>();
	}

	inline const glm::vec2& texcoord() const {
		return get<TEX_I>();
	}
};

struct Vertex4P : public Tuple<glm::vec4> {
	enum { POS_I = 0 };

	inline glm::vec4& position() {
		return get<POS_I>();
	}

	inline const glm::vec4& position() const {
		return get<POS_I>();
	}
};

struct Vertex4P3T : public Tuple<glm::vec4, glm::vec3> {
  enum { POS_I = 0, TEX_I = 1};

  inline glm::vec4& position() {
    return get<POS_I>();
  }

  inline glm::vec3& texcoord() {
    return get<TEX_I>();
  }

  inline const glm::vec4& position() const {
    return get<POS_I>();
  }

  inline const glm::vec3& texcoord() const {
    return get<TEX_I>();
  }
};

template <>
struct IsGfxTuple<Vertex4P3T> {
  static const constexpr bool value = true;
};

template <>
struct IsGfxTuple<Vertex4P3N2T> {
	static const constexpr bool value = true;
};

template <>
struct IsGfxTuple<Vertex4P2T> {
	static const constexpr bool value = true;
};

template <>
struct IsGfxTuple<Vertex4P3N> {
	static const constexpr bool value = true;
};

template <>
struct IsGfxTuple<Vertex4P> {
	static const constexpr bool value = true;
};
}

#endif /* GFX_UTILS_VERTEX_H_ */
