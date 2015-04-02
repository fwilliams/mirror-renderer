#include "sdl_gl_window.h"

#include <iostream>

using namespace std;

SDLGLWindow::SDLGLWindow(size_t width, size_t height) :
    vpx(width), vpy(height), running(false) {
  SDL_Init(SDL_INIT_EVERYTHING);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);

  window = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vpx, vpy,
      SDL_WINDOW_OPENGL);

  gl_ctx = SDL_GL_CreateContext(window);

  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &ctx_maj_version);
  SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &ctx_min_version);

  glewExperimental = GL_TRUE;
  GLenum glew_init_err = glewInit();
  if (glew_init_err != GLEW_OK) {
    throw GLEWInitFailedException(
        string("Error initializing GLEW: ")
            + reinterpret_cast<const char*>(glewGetErrorString(glew_init_err)));
  }
}

void SDLGLWindow::mainLoop() {
  running = true;

  setup(*this);
  do {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE) {
        running = false;
      }
      handle_event(*this, event);
    }
    update(*this);
    draw(*this);
    SDL_GL_SwapWindow(window);
    SDL_Delay(33);
  } while (running);

  teardown(*this);
  SDL_GL_DeleteContext(gl_ctx);
  SDL_DestroyWindow(window);
  SDL_Quit();
}


void gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* usr) {
  clog << msg << endl;
}

void SDLGLWindow::enableDebugLogging()  {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_callback, nullptr);
}
