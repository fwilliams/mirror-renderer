#include <GL/glew.h>

#include "utils/glm_traits.h"

#ifndef UTILS_VAO_GENERATOR_H_
#define UTILS_VAO_GENERATOR_H_

struct VAOGenerator {
private:

  template <typename T, class... Rest>
  struct EnableArrayElem {
    static void enable(GLuint index, size_t stride, size_t offset) {

      EnableArrayElem<T>::enable(index, stride, offset);
      EnableArrayElem<Rest...>::enable(index+1, stride, offset + sizeof(T));
    }
  };

  template <typename T>
  struct EnableArrayElem<T> {
    static void enable(GLuint index, size_t stride, size_t offset) {
//      std::cout <<
//          "binding attrib index " << index <<
//          " of size " << sizeof(T) <<
//          " with stride " << stride <<
//          ", offset " << offset <<
//          ", dim " << glutils::dim<T>() <<
//          ", and typeid == float? = " << (glutils::gl_type_id<T>() == GL_FLOAT) << std::endl;
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index, utils::dim<T>(), utils::gl_value_type_id<T>(), GL_FALSE, stride, (void*)offset);
    }
  };

public:
  template <class... Attribs>
  inline static GLuint generateAttribArray(size_t offset) {
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    const size_t sizeofVertex = sizeof(std::tuple<Attribs...>);
    EnableArrayElem<Attribs...>::enable(0, sizeofVertex, offset);
    return vao;
  }
};


#endif /* UTILS_VAO_GENERATOR_H_ */
