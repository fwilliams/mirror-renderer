#include <GL/glew.h>

#include <tuple>
#include <iostream>

#include "glm_traits.h"

#ifndef GL_HELPER_H_
#define GL_HELPER_H_

const char* get_gl_error_string(GLenum err);

void check_SDL_error(int line = -1);

#ifndef NDEBUG
#define check_gl_error() \
  { \
  GLenum err = glGetError(); \
  if (err != GL_NO_ERROR) { \
    fprintf(stderr, "GL Error <%s. %d>: %s\n", __FILE__, __LINE__, get_gl_error_string(err));\
    /*exit(err);*/ \
  } \
  }
#else
#define check_gl_error()
#endif

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
      glEnableVertexAttribArray(index);
      glVertexAttribPointer(index, dim<T>(), gl_type_id<T>(), GL_FALSE, stride, (void*)offset);
      std::cout << "index " << index << " is at offset " << offset << std::endl;
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

#endif /* GL_HELPER_H_ */
