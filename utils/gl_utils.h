#include <GL/glew.h>

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



#endif /* GL_HELPER_H_ */
