#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "renderer/camera.h"
#include "renderer/renderer.h"
#include "utils/interactive_window.h"

using namespace glm;
using namespace std;

struct SimpleMirrorGLWindow : InteractiveGLWindow {
  const float FOV_Y = 45.0f;

  // Geometry for a cube, a plane, and a sphere
  Geometry cubeMesh;
  Geometry planeMesh;
  Geometry sphereMesh;

  // Materials for the sphere and cube
  shared_ptr<Material> sphereMaterial;
  shared_ptr<Material> cubeMaterial;
  shared_ptr<Material> planeMaterial;

  // Transformations for geometry in the scene
  mat4 cubeTransform = scale(translate(mat4(1.0), vec3(-4.0, 1.0, -4.0)), vec3(2.0));
  mat4 sphereTransform = scale(translate(mat4(1.0), vec3(0.0, 1.5, 0.0)), vec3(1.0, 1.0, 1.0));
  mat4 planeTransform = scale(rotate(mat4(1.0), glm::half_pi<float>(), vec3(1.0, 0.0, 0.0)), vec3(10000.0));

  GLuint cubemapTex = 0;

  SimpleMirrorGLWindow(size_t w, size_t h) :
      InteractiveGLWindow(w, h) {
  }

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
    rndr.enableDepthBuffer();
    rndr.enableFaceCulling();

    // Create geometry
    cubeMesh = Geometry::make_cube(vec3(1.0), false);
    sphereMesh = Geometry::make_sphere(1.5, 100, 100);
    planeMesh = Geometry::make_plane(100, 100);

    // Setup the camera
    camera().setPosition(vec3(0.0, 1.0, -7.5));
    camera().setPerspectiveProjection(FOV_Y, aspectRatio(), 0.5, 1000.0);
    camera().setCameraVelocity(vec2(0.1));


    // Setup a grid of 9 lights above the center of the ball and one light along the +z axis
    Light l1 = {vec4(0.0),
                vec4(0.5, 0.5, 0.5, 1.0)};
    vec4 center(0.0, 15.0, -5.0, 1.0);
    vec2 squareSize(42.5);
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        size_t light_index = 3 * i + j;
        vec4 offset((i-1)*squareSize.x/2.0, 0.0, (j-1)*squareSize.y/2.0, 0.0);
        rndr.enableLight(light_index);
        rndr.setLight(light_index, l1);
        rndr.setLightPos(light_index, (center + offset));
        rndr.setLightAttenuation(light_index, 25.0);
      }
    }

    Light l2 = {vec4(0.0, 2.0, -5.0, 1.0),
                vec4(0.3525, 0.3525, 0.3525, 1.0)};
    rndr.enableLight(9);
    rndr.setLight(9, l2);
    rndr.setLightAttenuation(9, 20.0);

    rndr.setGlobalAmbient(vec4(0.005, 0.005, 0.005, 1.0));

    vec3 goldColor(1.0, 0.71, 0.29);

    planeMaterial = rndr.createMaterial(vec3(0.05), vec3(0.025), 0.8f, 0.8f);
    sphereMaterial = rndr.createMaterial(goldColor, goldColor * 0.5f, 0.2f, 0.8f);
    cubeMaterial = rndr.createMaterial(goldColor, goldColor * 0.5f, 0.2f, 0.8f);
  }

  void onDraw(Renderer& rndr) {
    rndr.clearViewport();
    rndr.startFrame();

    // Draw the and the cube sphere
    rndr.setMaterial(planeMaterial);
    rndr.draw(planeMesh, planeTransform);

    rndr.setMaterial(cubeMaterial);
    rndr.draw(cubeMesh, cubeTransform);

    rndr.setMaterial(sphereMaterial);
    rndr.draw(sphereMesh, sphereTransform);

    // Draw a cube over each light
    rndr.drawLights(cubeMesh, scale(mat4(1.0), vec3(0.2)));
  }

  void teardown(SDLGLWindow& w) {
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
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(800, 600);
  w.mainLoop();
}
