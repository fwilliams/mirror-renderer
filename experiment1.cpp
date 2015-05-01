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
  // Renderer to render objects in the scene
  Renderer* rndr = nullptr;

  // Camera and information to make an FPS camera
  Camera camera;
  const float FOV_Y = 45.0f;
  vec2 moveCamera = vec2(0, 0);
  vec2 cameraSphericalCoords;
  vec2 cameraVelocity;
  vec2 cameraAngularVel;


  // Geometry for a cube and a sphere
  Geometry sphereMesh;
  Geometry cubeMesh;

  // Materials for the sphere and cube
  Material sphereMaterial;
  Material cubeMaterial;

  // Transformations for geometry in the scene
  mat4 cubeTransform = scale(translate(mat4(1.0), vec3(0.0, 4.0, 0.0)), vec3(4.0, 8.0, 4.0));
  mat4 sphereTransform = translate(mat4(1.0), vec3(-5.0, 3.5, -4.0));

  // Programs used to render objects in the scene
  GLuint phongProgram = 0, drawNormalsProgram = 0, drawLightsProgram = 0, drawSkyboxProgram = 0;
  // Locations of uniforms in programs
  static const GLuint MATERIAL_LOC = 3;
  static const GLuint COLOR1_LOC = 2;
  static const GLuint COLOR2_LOC = 3;

  SimpleMirrorGLWindow(size_t w, size_t h) :
      SDLGLWindow(w, h) {
  }

  void drawNormals(const vec4& color1, const vec4& color2) {
    rndr->setProgram(drawNormalsProgram);
    glUniform4fv(COLOR1_LOC, 1, glm::value_ptr(color1));
    glUniform4fv(COLOR2_LOC, 1, glm::value_ptr(color2));
    rndr->draw(sphereMesh.normal_view_vao, sphereMesh.num_vertices * 2, sphereTransform);
    rndr->draw(cubeMesh.normal_view_vao, cubeMesh.num_vertices * 2, cubeTransform);
    for(size_t i = 0; i < rndr->numLights(); i++) {
      const mat4 lightTransform = scale(translate(mat4(1.0), rndr->lightPosition(i)), vec3(0.1));
      rndr->draw(cubeMesh.normal_view_vao, (size_t)(cubeMesh.num_vertices * 2), lightTransform);
    }
  }

  void setMaterial(const Material& material) {
    glUniform4fv(MATERIAL_LOC, 1, value_ptr(material.diffuse));
    glUniform4fv(MATERIAL_LOC + 1, 1, value_ptr(material.specular));
    glUniform1f(MATERIAL_LOC + 2, material.shine);
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
    dCamSphericalPos = cameraAngularVel * radians(dCamSphericalPos);
    cameraSphericalCoords += dCamSphericalPos;

    // Don't let the user rotate up and down more than 90 degrees
    cameraSphericalCoords.x = clamp(cameraSphericalCoords.x, -half_pi<float>(), half_pi<float>());

    // Keep the camera y angle in the range (0, 360) degrees to prevent floating point errors
    cameraSphericalCoords.y = mod(cameraSphericalCoords.y, two_pi<float>());

    // Set the orientation of the camera based on the stored angles
    camera.setRotationAngles(vec3(cameraSphericalCoords.x, cameraSphericalCoords.y, 0.0));
  }

  void setup(SDLGLWindow& w) {
    // Initialize the renderer
    rndr = new Renderer();
    rndr->setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
    rndr->enableDepthBuffer();
    rndr->enableFaceCulling();


    // Build shader programs
    phongProgram = ProgramBuilder::buildFromFiles("shaders/phong_vertex.glsl",
                                                  "shaders/phong_frag.glsl");

    // Make debug program which draws the normals of a piece of geometry
    drawNormalsProgram = ProgramBuilder::buildFromFiles("shaders/draw_normals_vert.glsl",
                                                        "shaders/draw_normals_frag.glsl");

    // Make program to draw lights
    drawLightsProgram = ProgramBuilder::buildFromFiles("shaders/draw_lights_vert.glsl",
                                                       "shaders/draw_lights_frag.glsl");


    // Create geometry
    cubeMesh = Geometry::make_cube(vec3(1.0), false);
    sphereMesh = Geometry::make_sphere(1.5, 100, 100);


    // Setup the camera
    camera.setPosition(vec3(0.0, 1.0, -7.5));
    camera.setPerspectiveProjection(FOV_Y, w.aspectRatio(), 0.5, 1000.0);
    cameraVelocity = vec2(0.1);
    cameraAngularVel = vec2(3.0);


    // Setup a grid of 9 lights above the center of the ball and one light along the +z axis
    Light l1 = {vec4(0.0),
                vec4(0.075, 0.075, 0.125, 1.0),
                vec4(0.325, 0.325, 0.325, 1.0)};
    vec4 center(0.0, 15.0, -5.0, 1.0);
    vec2 squareSize(42.5);
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        size_t light_index = 3 * i + j;
        vec4 offset((i-1)*squareSize.x/2.0, 0.0, (j-1)*squareSize.y/2.0, 0.0);
        rndr->enableLight(light_index);
        rndr->setLight(light_index, l1);
        rndr->setLightPos(light_index, (center + offset));
        rndr->setLightAttenuation(light_index, 25.0);
      }
    }

    Light l2 = {vec4(0.0, 2.0, -5.0, 1.0),
                vec4(0.3525, 0.4525, 0.3525, 1.0),
                vec4(0.1, 0.1, 0.1, 1.0) };
    rndr->enableLight(9);
    rndr->setLight(9, l2);
    rndr->setLightAttenuation(9, 20.0);

    rndr->setGlobalAmbient(vec4(0.05, 0.05, 0.05, 1.0));


    // Setup materials
    cubeMaterial.diffuse = vec4(0.2, 0.2, 0.5, 1.0);
    cubeMaterial.specular = vec4(0.3, 0.2, 0.15, 1.0);
    cubeMaterial.shine = 256.0f;

    sphereMaterial.diffuse = vec4(0.35, 0.2, 0.25, 1.0);
    sphereMaterial.specular = vec4(0.25, 0.3, 0.15, 1.0);
    sphereMaterial.shine = 1024.0f;


    // Move mouse cursor to the middle of the screen and hide it
    setMousePosition(w.width()/2, w.height()/2);
    showCursor(false);
  }

  void handle_event(SDLGLWindow& w, const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_w) {
        cameraVelocity.y = 0.1;
        moveCamera.y = 1.0;
      }
      if(event.key.keysym.sym == SDLK_s) {
        moveCamera.y = -1.0;
      }
      if(event.key.keysym.sym == SDLK_d) {
        moveCamera.x = 1.0;
      }
      if(event.key.keysym.sym == SDLK_a) {
        moveCamera.x = -1.0;
      }
    }

    if(event.type == SDL_KEYUP) {
      if(event.key.keysym.sym == SDLK_w) {
        moveCamera.y = 0.0;
      }
      if(event.key.keysym.sym == SDLK_s) {
        moveCamera.y = 0.0;
      }
      if(event.key.keysym.sym == SDLK_d) {
        moveCamera.x = 0.0;
      }
      if(event.key.keysym.sym == SDLK_a) {
        moveCamera.x = 0.0;
      }
    }
  }

  void update(SDLGLWindow& w) {
    updateCameraOrientation();
    camera.advance(moveCamera.y * cameraVelocity.y);
    camera.strafeRight(moveCamera.x * cameraVelocity.x);
    rndr->setProjectionMatrix(camera.getProjectionMatrix());
    rndr->setViewMatrix(camera.getViewMatrix());
  }

  void draw(SDLGLWindow& w) {
    rndr->clearViewPort();
    rndr->startFrame();


    // Draw the and the cube sphere
    rndr->setProgram(phongProgram);

    setMaterial(cubeMaterial);
    rndr->draw(cubeMesh, cubeTransform);

    setMaterial(sphereMaterial);
    rndr->draw(sphereMesh, sphereTransform);


    // Draw a cube over each light
    rndr->setProgram(drawLightsProgram);
    for(size_t i = 0; i < rndr->numLights(); i++) {
      glUniform1ui(1, i);
      rndr->draw(cubeMesh, scale(translate(mat4(1.0), rndr->lightPosition(i)), vec3(0.1)));
    }
  }

  void teardown(SDLGLWindow& w) {
    delete rndr;

    glDeleteBuffers(1, &sphereMesh.vbo);
    glDeleteBuffers(1, &sphereMesh.ibo);
    glDeleteBuffers(1, &sphereMesh.normal_view_vbo);
    glDeleteVertexArrays(1, &sphereMesh.vao);
    glDeleteVertexArrays(1, &sphereMesh.normal_view_vao);

    glDeleteBuffers(1, &cubeMesh.vbo);
    glDeleteBuffers(1, &cubeMesh.ibo);
    glDeleteBuffers(1, &cubeMesh.normal_view_vbo);
    glDeleteVertexArrays(1, &cubeMesh.vao);
    glDeleteVertexArrays(1, &cubeMesh.normal_view_vao);

    glDeleteProgram(phongProgram);
    glDeleteProgram(drawLightsProgram);
    glDeleteProgram(drawNormalsProgram);
    glDeleteProgram(drawSkyboxProgram);
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(1024, 768);
  w.mainLoop();
}
