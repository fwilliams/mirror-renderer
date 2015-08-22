#include <iostream>
#include <tuple>
#include <unordered_map>
#include <functional>
#include <climits>
#include <type_traits>
#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/gl_utils.h"
#include "utils/interactive_window.h"

#include "geometry/tile_mesh.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

using RenderMesh = TileMesh<TEXTURED, QuadPlanarTileSet>;

class App: public InteractiveGLWindow {
  GLProgramBuilder programBuilder;

  GLuint renderProgram = 0;
  GLuint solidColorProgram = 0;

  unique_ptr<RenderMesh> tileMesh;

  Mode mode;

  class Config {
    size_t mirrorViewId = 0;
    std::array<Quad4, 4> mirrorFaces = RenderMesh::edgeFaces();
  public:
    bool showWireFrame = false;

    Quad4 currentMirror() {
      return mirrorFaces[mirrorViewId];
    }

    void nextMirror() {
      mirrorViewId = (mirrorViewId + 1) % 4;
    }
  } config;

public:
  App(unsigned w, unsigned h, Mode mode = TEXTURED) : InteractiveGLWindow(w, h), mode(mode) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
    rndr.enableFaceCulling();

    programBuilder.addIncludeDir("shaders/glsl330");
    programBuilder.addIncludeDir("shaders");

    if(mode == IDENTIFIED) {
      rndr.enableDepthBuffer();

      renderProgram = programBuilder.buildFromFiles(
          "shaders/tile_color_vert.glsl",
          "shaders/tile_color_frag.glsl");

    } else if(mode == TEXTURED) {
      rndr.enableAlphaBlending();
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

      renderProgram = programBuilder.buildFromFiles(
          "shaders/solid_texture_vert.glsl",
          "shaders/solid_texture_frag.glsl");
    }

    solidColorProgram = programBuilder.buildFromFiles(
        "shaders/solid_color_vert.glsl",
        "shaders/solid_color_frag.glsl");

    tileMesh = make_unique<RenderMesh>(5);
  }

  void onUpdate() {
    vec4 p[2] {config.currentMirror().p1, config.currentMirror().p3};
    for(size_t i = 0; i < 2; i++) {
      p[i] = camera().getViewMatrix() * p[i];
      p[i] /= 2.0;
    }

    camera().setPerspectiveProjection(p[0].x, p[1].x, p[1].y, p[0].y, 1.0 - p[0].z, 10000.0);
  }

  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_t)) {
        config.showWireFrame = !config.showWireFrame;
    }
    if(isKeyDownEvent(evt, SDLK_v)) {
      config.nextMirror();

      // Rotate the camera to face the mirror
      Quad4 q = config.currentMirror();
      float angle = acos(dot(vec3(0.0, 0.0, 1.0), q.normal())); // angle between the z axis (default view) and mirror plane
      if(dot(cross(vec3(0.0, 0.0, 1.0), q.normal()), vec3(0.0, 1.0, 0.0)) > 0) { angle = -angle; } // Add sign to the angle
      camera().transformView(rotate(mat4(1.0), angle, vec3(0.0, 1.0, 0.0))); // Transform the view
    }
  }

  void onDraw(Renderer& rndr) {
    rndr.clearViewport();

    rndr.setProgram(renderProgram);

    float f = 1.0; // TODO: I think this is a problem and we should have an f for each mirror
    vec3 c = camera().getPosition();
    float fMinusZc = f - c.z;
    mat4 reprojectionMat = mat4(vec4(fMinusZc, 0, 0, 0),
                                vec4(0, fMinusZc, 0, 0),
                                vec4(c.x, c.y, f, 1),
                                vec4(-f*c, -c.z));

//    mat4 reprojectionMat = mat4(vec4(fMinusZc, 0,        c.x, -f*c.x),
//                                vec4(0,        fMinusZc, c.y, -f*c.y),
//                                vec4(0,        0,        f,   -f*c.z),
//                                vec4(0,        0,        1,   -c.z));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileTextureArray());
    glUniform1i(glGetUniformLocation(renderProgram, "texid"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileDepthTextureArray());
    glUniform1i(glGetUniformLocation(renderProgram, "depthId"), 1);

    glUniformMatrix4fv(glGetUniformLocation(renderProgram, "reprojMat"), 1, GL_FALSE, value_ptr(reprojectionMat));
    rndr.draw(tileMesh->geometry(), mat4(1.0), PrimitiveType::TRIANGLES);

    if(config.showWireFrame) {
      glLineWidth(3.0);
      rndr.setProgram(solidColorProgram);
      glUniform4fv(glGetUniformLocation(solidColorProgram, "color"), 1, value_ptr(vec4(0.0, 1.0, 0.0, 1.0)));
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      rndr.draw(tileMesh->geometry(), scale(mat4(1.0), vec3(1.0)), PrimitiveType::TRIANGLES);
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }
};

int main(int argc, char** argv) {
  if(argc > 2 && strcmp(argv[1], "-p") == 0) {
    size_t radius = atoi(argv[2]);
    RenderMesh tileMesh(radius);
    tileMesh.printTextureNames();
  } else {
    App w(800, 600);
    w.mainLoop();
  }
}
