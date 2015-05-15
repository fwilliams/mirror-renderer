#include <iostream>
#include <tuple>

#include <glm/glm.hpp>

#include "utils/gl_utils.h"
#include "utils/sdl_gl_window.h"
#include "utils/gl_program_builder.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"

using namespace std;
using namespace glm;

//template <typename... Attribs>
//Geometry generateTileGeometry(PlanarTileSet<PlanarTileType::QUAD>& tiles) {
//  const size_t numVertices = 0;
//  const size_t numIndices = numVertices;
//  Geometry ret = Geometry::fromVertexAttribs<vec4, Attribs...>(numVertices, numIndices);
//
//  typedef tuple<vec4, Attribs...> vertex;
//
//  glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
//  vertex* verts = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
//
//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
//  GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));
//
//  return ret;
//}

class ProjectedTileVizWindow: public SDLGLWindow {
  const unsigned NUM_COORDS_W = 55;
  const unsigned NUM_COORDS_H = 55;

  Renderer* rndr = nullptr;

  Geometry grid;

  GLuint program = 0;

  FirstPersonCamera camera;

  typedef tuple<vec4, vec4> vertex;

public:
  ProjectedTileVizWindow(unsigned w, unsigned h) : SDLGLWindow(w, h) {}

  Geometry makeGrid() {
    size_t numVertices = NUM_COORDS_W * NUM_COORDS_H * 2;
    size_t numIndices = (NUM_COORDS_W-1) * (NUM_COORDS_H - 1) * 10 + (NUM_COORDS_W-1) * 6 + (NUM_COORDS_H - 1)*6;
    Geometry ret = Geometry::fromVertexAttribs<vec4, vec4>(numVertices, numIndices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* verts = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));
    check_gl_error();

    for(size_t i = 0; i < NUM_COORDS_W; i++) {
      for(size_t j = 0; j < NUM_COORDS_H * 2; j += 2) {
        vec4 pos(-static_cast<float>(NUM_COORDS_W)/2.0 + i, -0.5,
                 -static_cast<float>(NUM_COORDS_H)/2.0 + static_cast<float>(j)/2.0, 1.0);
        vec4 color(static_cast<float>(i) / NUM_COORDS_W, static_cast<float>(j/2) / NUM_COORDS_H, 0.0, 1.0);
        const size_t offset = i * NUM_COORDS_W * 2 + j;
        verts[offset] = make_tuple(color, pos);

        pos.y = 0.5;
        verts[offset + 1] = make_tuple(color, pos);
      }
    }

    size_t w = 0;
    for(size_t i = 0; i < NUM_COORDS_W - 1; i++) {
      for(size_t j = 0; j < NUM_COORDS_H - 1; j++) {
        const GLuint offset = i * NUM_COORDS_W * 2 + j * 2;
        inds[w++] = offset;
        inds[w++] = offset + 1;

        inds[w++] = offset;
        inds[w++] = offset + 2;

        inds[w++] = offset;
        inds[w++] = offset + NUM_COORDS_W*2;

        inds[w++] = offset + 1;
        inds[w++] = offset + 3;

        inds[w++] = offset + 1;
        inds[w++] = offset + 1 + NUM_COORDS_W*2;
      }
    }
    for(size_t i = 0; i < NUM_COORDS_W-1; i++) {
      const GLuint offset = NUM_COORDS_W * 2 * (NUM_COORDS_H-1) + i * 2;
      inds[w++] = offset;
      inds[w++] = offset + 1;
      inds[w++] = offset;
      inds[w++] = offset + 2;
      inds[w++] = offset + 1;
      inds[w++] = offset + 3;
    }

    for(size_t i = 0; i < NUM_COORDS_H-1; i++) {
      const GLuint offset = (i+1) * NUM_COORDS_W * 2 - 2;
      inds[w++] = offset;
      inds[w++] = offset + 1;
      inds[w++] = offset;
      inds[w++] = offset + NUM_COORDS_W * 2;
      inds[w++] = offset + 1;
      inds[w++] = offset + NUM_COORDS_W * 2 + 1;
    }
    inds[w++] = NUM_COORDS_W * NUM_COORDS_H * 2 - 2;
    inds[w++] = NUM_COORDS_W * NUM_COORDS_H * 2 - 1;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  void handle_event(SDLGLWindow& w, const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_w) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::POSITIVE);
      }
      if(event.key.keysym.sym == SDLK_s) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
      }
      if(event.key.keysym.sym == SDLK_d) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::POSITIVE);
      }
      if(event.key.keysym.sym == SDLK_a) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
      }
    }

    if(event.type == SDL_KEYUP) {
      if(event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_s) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::STOPPED);
      }
      if(event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::STOPPED);
      }
    }
  }

  void update(SDLGLWindow& w) {
    vec2 mp;
    getNormalizedMousePos(value_ptr(mp));
    setMousePosition(width()/2, height()/2);
    camera.updateLookat(mp);
    camera.updatePosition();
    rndr->setProjectionMatrix(camera.getProjectionMatrix());
    rndr->setViewMatrix(camera.getViewMatrix());
  }


  void setup(SDLGLWindow& w) {
    rndr = new Renderer();
    rndr->setClearColor(vec4(0.1, 0.1, 0.1, 1.0));

    camera.setPosition(vec3(0.0, 0.0, 0.0));
    camera.setPerspectiveProjection(45.0, w.aspectRatio(), 0.5, 10000.0);
    camera.setCameraVelocity(vec2(0.9));

    program = GLProgramBuilder::buildFromFiles("shaders/flat_color_vert.glsl",
                                               "shaders/flat_color_frag.glsl");

    grid = makeGrid();

    setMousePosition(w.width()/2, w.height()/2);
    showCursor(false);
  }

  void draw(SDLGLWindow& w) {
    rndr->clearViewport();
    rndr->startFrame();
    glPointSize(3.0);
    glLineWidth(1.0);
    rndr->setProgram(program);
    rndr->draw(grid, scale(mat4(1.0), vec3(100.0)), Renderer::PrimitiveType::LINES);
  }
};

int main(int argc, char** argv) {
  ProjectedTileVizWindow w(1024, 1024);
  w.mainLoop();
}