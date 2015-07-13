#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <string>
#include <iostream>
#include <stdexcept>
#include <functional>

#ifndef SDL_GL_HELPER_H_
#define SDL_GL_HELPER_H_

class SDLGLWindow {
  size_t vpx;
  size_t vpy;

  SDL_Window* window;

  SDL_GLContext gl_ctx;

  int ctx_maj_version, ctx_min_version;

  bool running;

public:
  struct GLEWInitFailedException: public std::runtime_error {
    GLEWInitFailedException(const std::string& arg) :
      std::runtime_error(arg) {
    }
  };

  size_t width() const {
    return vpx;
  }

  size_t height() const {
    return vpy;
  }

  double aspectRatio() const {
    return static_cast<float>(width()) / height();
  }

  SDLGLWindow(size_t width, size_t height);

  virtual void setup(SDLGLWindow&) {};

  virtual void handle_event(SDLGLWindow&, const SDL_Event& event) {};

  virtual void update(SDLGLWindow&) {};

  virtual void draw(SDLGLWindow&) {};

  virtual void teardown(SDLGLWindow&) {};

  void enableDebugLogging();

  void mainLoop();

  void showCursor(bool visible);

  void setMousePosition(int x, int y);

  void setFullScreen(bool fullscreen);

  void close() {
    running = false;
  }

  void getNormalizedMousePos(float pos[2]);

  void getNormalizedMousePos(float& x, float& y);

  virtual ~SDLGLWindow();
};

#endif /* SDL_GL_HELPER_H_ */

