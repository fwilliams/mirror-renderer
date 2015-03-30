#include <SDL2/SDL.h>

#include "gl_utils.h"

const char* get_gl_error_string(GLenum err) {
  switch (err) {
  case GL_NO_ERROR:
    return "GL_NO_ERROR";
  case GL_INVALID_OPERATION:
    return "INVALID_OPERATION";
  case GL_INVALID_ENUM:
    return "INVALID_ENUM";
  case GL_INVALID_VALUE:
    return "INVALID_VALUE";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "OUT_OF_MEMORY";
  case GL_STACK_UNDERFLOW:
    return "STACK_UNDERFLOW";
  case GL_STACK_OVERFLOW:
    return "STACK_OVERFLOW";
  default:
    return "";
  }
}

void check_SDL_error(int line) {
#ifndef NDEBUG
  const char *error = SDL_GetError();
  if (*error != '\0') {
    if (line != -1) {
      fprintf(stderr, "SDL Error: %s\n", error);
    } else {
      fprintf(stderr, "SDL Error <%s. %d>: %s\n", __FILE__, __LINE__, error);
    }
    SDL_ClearError();
  }
#endif
}
