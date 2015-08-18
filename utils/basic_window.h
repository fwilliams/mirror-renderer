#include "sdl_gl_window.h"

#ifndef UTILS_BASIC_WINDOW_H_
#define UTILS_BASIC_WINDOW_H_

class BasicGLWindow : public detail::SDLGLWindow {
	std::unique_ptr<Renderer> mRenderer = nullptr;

	void handle_event(SDLGLWindow& w, const SDL_Event& event) {
		onEvent(event);
	}

	void setup(SDLGLWindow& w) {
		onCreate(*mRenderer);
	}

	void draw(SDLGLWindow& w) {
		onDraw(*mRenderer);
	}

public:
	BasicGLWindow(size_t w, size_t h) : detail::SDLGLWindow(w, h) {
		mRenderer = std::make_unique<Renderer>();
	}

	virtual void onCreate(Renderer& rndr) {}

	virtual void onEvent(const SDL_Event& event) {}

	virtual void onDraw(Renderer& rndr) {}

};


#endif /* UTILS_BASIC_WINDOW_H_ */
