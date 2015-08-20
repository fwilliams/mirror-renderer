#include <memory>
#include <iostream>

#include <glm/glm.hpp>

#include "renderer/camera.h"

#include "renderer/renderer.h"

#include "sdl_gl_window.h"

#ifndef INTERACTIVE_GL_WINDOW_H_
#define INTERACTIVE_GL_WINDOW_H_

class InteractiveGLWindow : public detail::SDLGLWindow {
	FirstPersonCamera mCamera;

	std::unique_ptr<Renderer> mRenderer = nullptr;

	void update(SDLGLWindow& w) {
		glm::vec2 mp;
		getNormalizedMousePos(value_ptr(mp));
		setMousePosition(width()/2, height()/2);
		mCamera.updateLookat(mp);
		mCamera.updatePosition();
		mRenderer->setProjectionMatrix(mCamera.getProjectionMatrix());
		mRenderer->setViewMatrix(mCamera.getViewMatrix());

		onUpdate();
	}

	void handle_event(SDLGLWindow& w, const SDL_Event& event) {
		if(event.type == SDL_KEYDOWN) {
			if(event.key.keysym.sym == SDLK_w) {
				mCamera.setForwardDirection(FirstPersonCamera::CameraDirection::POSITIVE);
			}
			if(event.key.keysym.sym == SDLK_s) {
				mCamera.setForwardDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
			}
			if(event.key.keysym.sym == SDLK_d) {
				mCamera.setHorizontalDirection(FirstPersonCamera::CameraDirection::POSITIVE);
			}
			if(event.key.keysym.sym == SDLK_a) {
				mCamera.setHorizontalDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
			}
		}

		if(event.type == SDL_KEYUP) {
			if(event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_s) {
				mCamera.setForwardDirection(FirstPersonCamera::CameraDirection::STOPPED);
			}
			if(event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a) {
				mCamera.setHorizontalDirection(FirstPersonCamera::CameraDirection::STOPPED);
			}
		}

		if(event.type == SDL_MOUSEBUTTONDOWN) {
			if(event.button.button == SDL_BUTTON_LEFT) {
				std::cout << to_string(mCamera.getLookatVector()) << std::endl;
			}
		}

		onEvent(event);
	}

	void setup(SDLGLWindow& w) {
		onCreate(*mRenderer);
	}

	void draw(SDLGLWindow& w) {
		onDraw(*mRenderer);
	}

public:
	InteractiveGLWindow(size_t w, size_t h) : detail::SDLGLWindow(w, h) {
		mRenderer = std::make_unique<Renderer>();

		mCamera.setPerspectiveProjection(45.0, aspectRatio(), 0.1, 10000.0);
		mCamera.setCameraVelocity(glm::vec2(0.005));

		setMousePosition(width()/2, height()/2);
		showCursor(false);
	}

	virtual void onCreate(Renderer& rndr) {}

	virtual void onEvent(const SDL_Event& event) {}

	virtual void onDraw(Renderer& rndr) {}

	virtual void onUpdate() {}

	FirstPersonCamera& camera() {
		return mCamera;
	}
};




#endif /* INTERACTIVE_SDL_WINDOW_H_ */
