#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "renderer.h"
#include "camera.h"
#include "utils/sdl_gl_window.h"
#include "utils/gl_program_builder.h"
#include "utils/gl_utils.h"

using namespace glm;
using namespace std;

struct Material {
  vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
  float shine = 0.0;
};

struct SimpleMirrorGLWindow: SDLGLWindow {
  Camera camera;

  const float FOV_Y = 45.0f;
  vec3 cameraVelocity;
  vec2 cameraSphericalCoords;

  Geometry sphere_mesh;
  GLuint phongProgram = 0;

  Material sphere_material;
  Renderer* rndr = nullptr;

  const GLuint MATERIAL_LOC = 3;

  SimpleMirrorGLWindow(size_t w, size_t h) :
      SDLGLWindow(w, h) {
  }

  void setup(SDLGLWindow& w) {
    // Initialize the renderer
    rndr = new Renderer();
    rndr->setClearColor(vec4(0.1, 0.1, 0.2, 1.0));
    rndr->enableDepthBuffer();


    // Build shader programs
    phongProgram = ProgramBuilder::buildFromFiles("shaders/phong_vertex.glsl",
                                                  "shaders/phong_frag.glsl");

    // Create sphere geometry
    sphere_mesh = Geometry::make_sphere(1.5, 55, 55);


    // Setup the camera
    camera.setPosition(vec3(0.0, 0.0, -4.5));
    camera.setPerspectiveProjection(FOV_Y, w.aspectRatio(), 0.5, 1000.0);

    // Setup a grid of 9 lights above the center of the ball and one light along the +z axis
    Light l1 = {vec4(0.0),
                vec4(0.15, 0.15, 0.15, 1.0),
                vec4(0.25, 0.25, 0.25, 1.0) };
    vec4 center(0.0, 3.0, 5.0, 1.0);
    vec2 squareSize(2.5);
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        vec4 offset((i-1)*squareSize.x/2.0, 0.0, (j-1)*squareSize.y/2.0, 0.0);
        rndr->setLight(3*i + j, l1);
        rndr->setLightPos(3*i + j, (center + offset));
      }
    }

    Light l2 = {vec4(0.0, 0.0, 15.0, 1.0),
                vec4(0.55, 0.95, 0.55, 1.0),
                vec4(0.1, 0.1, 0.1, 1.0) };
    rndr->setLight(9, l2);


    // Set uniforms for lighting program
    glUseProgram(phongProgram);

    // Setup material
    sphere_material.diffuse = vec4(0.4, 0.6, 0.7, 1.0);
    glUniform4fv(MATERIAL_LOC, 1, value_ptr(sphere_material.diffuse));

    sphere_material.specular = vec4(0.6, 0.4, 0.3, 1.0);
    glUniform4fv(MATERIAL_LOC + 1, 1, value_ptr(sphere_material.specular));

    sphere_material.shine = 250.0f;
    glUniform1f(MATERIAL_LOC + 2, sphere_material.shine);

    glUseProgram(0);

    setMousePosition(w.width()/2, w.height()/2);
    showCursor(false);
  }

  void updateCameraOrientation() {
    // Get the coordinates of the cursor with respect to the top left corner of the screen
    ivec2 mousePos;
    SDL_GetMouseState(&mousePos.x, &mousePos.y);

    // Reset the cursor position to the middle of the screen
    setMousePosition(width()/2, height()/2);

    // Transform mouse coordinates so (0, 0) is the center of the screen, y is up, and x is right
    mousePos = ivec2(-1, 1) * (mousePos - (ivec2(width(), height()) / 2));

    // Normalize mouse position
    const vec2 normalizedMousePos = vec2(mousePos.y, mousePos.x) / vec2(height(), width()) / 2.0f;
    vec2 dCamSphericalPos = normalizedMousePos * vec2(FOV_Y*aspectRatio(), FOV_Y) / 2.0f;
    dCamSphericalPos = radians(dCamSphericalPos);
    cameraSphericalCoords += dCamSphericalPos;

    // Don't let the user rotate up and down more than 90 degrees
    cameraSphericalCoords.x = clamp(cameraSphericalCoords.x, -half_pi<float>(), half_pi<float>());

    // Keep the camera y angle in the range (0, 360) degrees to prevent floating point errors
    cameraSphericalCoords.y = mod(cameraSphericalCoords.y, two_pi<float>());

    // Set the orientation of the camera based on the stored angles
    camera.setRotationAngles(vec3(cameraSphericalCoords.x, cameraSphericalCoords.y, 0.0));
  }

  void handle_event(SDLGLWindow& w, const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_w) {
        cameraVelocity.z = 0.1;
      }
      if(event.key.keysym.sym == SDLK_s) {
        cameraVelocity.z = -0.1;
      }
      if(event.key.keysym.sym == SDLK_d) {
        cameraVelocity.x = 0.1;
      }
      if(event.key.keysym.sym == SDLK_a) {
        cameraVelocity.x = -0.1;
      }
    }

    if(event.type == SDL_KEYUP) {
      if(event.key.keysym.sym == SDLK_w) {
        cameraVelocity.z = 0.0;
      }
      if(event.key.keysym.sym == SDLK_s) {
        cameraVelocity.z = 0.0;
      }
      if(event.key.keysym.sym == SDLK_d) {
        cameraVelocity.x = 0.0;
      }
      if(event.key.keysym.sym == SDLK_a) {
        cameraVelocity.x = 0.0;
      }
    }
  }

  void update(SDLGLWindow& w) {
    updateCameraOrientation();
    camera.advance(cameraVelocity.z);
    camera.strafeRight(cameraVelocity.x);
    rndr->setProjectionMatrix(camera.getProjectionMatrix());
    rndr->setViewMatrix(camera.getViewMatrix());
  }

  void teardown(SDLGLWindow& w) {
    delete rndr;
    glDeleteBuffers(1, &sphere_mesh.vbo);
    glDeleteBuffers(1, &sphere_mesh.ibo);
    glDeleteBuffers(1, &sphere_mesh.normal_view_vbo);
    glDeleteVertexArrays(1, &sphere_mesh.vao);
    glDeleteVertexArrays(1, &sphere_mesh.normal_view_vao);
    glDeleteProgram(phongProgram);
  }

  void draw(SDLGLWindow& w) {
    rndr->clearViewPort();
    rndr->startFrame();
    rndr->setProgram(phongProgram);
    rndr->draw(sphere_mesh);
    rndr->drawNormals(vec4(0.0, 1.0, 0.0, 1.0), vec4(0.0, 0.0, 1.0, 1.0), sphere_mesh);
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(1024, 768);
  w.mainLoop();
}
