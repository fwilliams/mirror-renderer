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

SDLGLWindow::~SDLGLWindow() {
  SDL_GL_DeleteContext(gl_ctx);
  SDL_DestroyWindow(window);
  SDL_Quit();
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
    SDL_Delay(15);
  } while (running);

  teardown(*this);
}

void SDLGLWindow::setMousePosition(int x, int y) {
  SDL_WarpMouseInWindow(window, x, y);
}

void SDLGLWindow::showCursor(bool toggle) {
  SDL_ShowCursor(toggle);
}

void gl_debug_callback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* msg, const void* usr) {
  clog << msg << endl;
}

void SDLGLWindow::enableDebugLogging()  {
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageCallback(gl_debug_callback, nullptr);
}


void SDLGLWindow::getNormalizedMousePos(float pos[2]) {
  // Get the coordinates of the cursor with respect to the top left corner of the screen
  int mousePos[2];
  SDL_GetMouseState(&mousePos[0], &mousePos[1]);

  // Transform mouse coordinates so (0, 0) is the center of the screen, y is up, and x is right
  float fMousePos[2];
  fMousePos[0] = -1.0f * (mousePos[0] - width()/2.0f);
  fMousePos[1] = mousePos[1] - height()/2.0f;

  // Normalize mouse position
  pos[1] = fMousePos[0] / (width()/2.0f);
  pos[0] = fMousePos[1] / (height()/2.0f);
}

void SDLGLWindow::getNormalizedMousePos(float& x, float& y) {
  // Get the coordinates of the cursor with respect to the top left corner of the screen
  int mousePos[2];
  SDL_GetMouseState(&mousePos[0], &mousePos[1]);

  // Transform mouse coordinates so (0, 0) is the center of the screen, y is up, and x is right
  float fMousePos[2];
  fMousePos[0] = -1.0f * (mousePos[0] - width()/2.0f);
  fMousePos[1] = mousePos[1] - height()/2.0f;

  // Normalize mouse position
  y = fMousePos[0] / (width()/2.0f);
  x = fMousePos[1] / (height()/2.0f);
}
