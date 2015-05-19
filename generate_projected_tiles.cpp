#include <iostream>
#include <tuple>
#include <functional>

#include <glm/glm.hpp>

#include "utils/gl_utils.h"
#include "utils/sdl_gl_window.h"
#include "utils/gl_program_builder.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"

#include "geometry/planar_tiling.h"

using namespace std;
using namespace glm;
using namespace geometry;

class ProjectedTileVizWindow: public SDLGLWindow {
  TriPlanarTileMapV<GLuint> tileMap;

  Renderer* rndr = nullptr;

  Geometry grid;

  GLuint program = 0;

  FirstPersonCamera camera;

  typedef tuple<vec4, vec4> vertex;

public:
  ProjectedTileVizWindow(unsigned w, unsigned h) : SDLGLWindow(w, h) {}

  template <typename Tiling>
  Geometry makeGrid(Tiling& tiling) {
    auto nearestN = [] (const glm::ivec2& tile) {
      return distance(Tiling::coords2d(tile), vec2(0)) <= 100;
    };

    using namespace std::placeholders;

    tiling.addTilesInNeighborhood(ivec2(0), nearestN);

    size_t numVertices = tiling.vertexCount();
    size_t numIndices = tiling.tileCount() * tiling.numVertsPerTile() * 2;

    Geometry ret = Geometry::fromVertexAttribs<vec4, vec4>(numVertices, numIndices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* verts = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    size_t vOffset = 0, iOffset = 0;
    for(auto i = tiling.vertices_begin(); i != tiling.vertices_end(); i++) {
      i->second.data = vOffset;
      vec2 pos = i->second.coords2d();
      verts[vOffset++] = make_tuple(vec4(1.0), vec4(pos.x, 0.0, pos.y, 1.0));
    }

    for(auto i = tiling.tiles_begin(); i != tiling.tiles_end(); i++) {
      for(auto v = i->second.vertices_begin(); v != i->second.vertices_end(); v++) {
        if(v == i->second.vertices_end() - 1) {
          inds[iOffset++] = (*v)->data;
          inds[iOffset++] = (*(i->second.vertices_begin()))->data;
        } else {
          inds[iOffset++] = (*v)->data;
          inds[iOffset++] = (*(v+1))->data;
        }
      }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  void setup(SDLGLWindow& w) {
    rndr = new Renderer();
    rndr->setClearColor(vec4(0.1, 0.1, 0.1, 1.0));

    camera.setPosition(vec3(0.5, 0.05, 0.5));
    camera.setPerspectiveProjection(45.0, w.aspectRatio(), 0.5, 10000.0);
    camera.setCameraVelocity(vec2(0.05));

    program = GLProgramBuilder::buildFromFiles("shaders/flat_color_vert.glsl",
                                               "shaders/flat_color_frag.glsl");

    grid = makeGrid(tileMap);

    setMousePosition(w.width()/2, w.height()/2);
    showCursor(false);
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

  void draw(SDLGLWindow& w) {
    rndr->clearViewport();
    rndr->startFrame();
    glPointSize(3.0);
    glLineWidth(1.0);
    rndr->setProgram(program);
    rndr->draw(grid, scale(mat4(1.0), vec3(1.0)), Renderer::PrimitiveType::LINES);
  }
};

int main(int argc, char** argv) {
  ProjectedTileVizWindow w(1024, 768);
  w.mainLoop();
}
