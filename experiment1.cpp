#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "renderer.h"
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
  Geometry sphere_mesh;
  GLuint phongProgram = 0;

  Material sphere_material;
  Renderer* rndr = nullptr;

  const GLuint MATERIAL_LOC = 3;
  const GLuint AMBIENT_LOC = 6;
  const GLuint LIGHTS_LOC = 7;

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


    // Setup view and projection transformations
    rndr->setPerspectiveProjection(45.0, w.aspectRatio(), 0.5, 1000.0);
    rndr->setViewLookat(vec3(0.0, 0.2, 4.5), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));


    // Setup a grid of 9 lights above the center of the ball and one light along the +z axis
    Light l1 = {vec4(0.0),
                vec4(0.15, 0.15, 0.15, 1.0),
                vec4(0.75, 0.75, 0.75, 1.0) };
    vec4 center(0.0, 55.0, 0.0, 1.0);
    vec2 squareSize(30.0);
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        vec4 offset((i-1)*squareSize.x/2.0, 0.0, (j-1)*squareSize.y/2.0, 0.0);
        rndr->setLight(3*i + j, l1);
        rndr->setLightPos(3*i + j, rndr->view() * (center + offset));
      }
    }

    Light l2 = {rndr->view() * vec4(0.0, 0.0, 15.0, 1.0),
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

    sphere_material.shine = 500.0f;
    glUniform1f(MATERIAL_LOC + 2, sphere_material.shine);

    // Setup uniforms for drawing normals
    glUseProgram(0);
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
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(1024, 768);
  w.mainLoop();
}
