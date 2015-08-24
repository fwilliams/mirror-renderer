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

using RenderMesh = TileMesh<QuadPlanarTileSet>;

class App: public InteractiveGLWindow {
  GLProgramBuilder programBuilder;

  GLuint drawSceneProgram = 0;
  GLuint vcLookupProgram = 0;
  GLuint solidColorProgram = 0;

  GLuint vcLookubFramebuffer = 0;
  GLuint vcLookupTexture = 0;

  ivec2 ctr = ivec2(0);

  unique_ptr<RenderMesh> tileMesh;

  float np = 0;

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
  App(unsigned w, unsigned h) : InteractiveGLWindow(w, h) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();

    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    programBuilder.addIncludeDir("shaders/glsl330");
    programBuilder.addIncludeDir("shaders");

    vcLookupProgram = programBuilder.buildFromFiles(
        "shaders/tile_color_vert.glsl",
        "shaders/tile_color_frag.glsl");

    drawSceneProgram = programBuilder.buildFromFiles(
        "shaders/solid_texture_vert.glsl",
        "shaders/solid_texture_frag.glsl");

    solidColorProgram = programBuilder.buildFromFiles(
        "shaders/solid_color_vert.glsl",
        "shaders/solid_color_frag.glsl");

    glGenFramebuffers(1, &vcLookubFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, vcLookubFramebuffer);

    glGenTextures(1, &vcLookupTexture);
    glBindTexture(GL_TEXTURE_2D, vcLookupTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width(), height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width(), height());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, vcLookupTexture, 0);
    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

    tileMesh = make_unique<RenderMesh>(2);
  }

  void onUpdate() {
    vec4 p[2] {config.currentMirror().p1, config.currentMirror().p3};
    for(size_t i = 0; i < 2; i++) {
      p[i] = camera().getViewMatrix() * p[i];
      p[i] /= 2.0;
    }
    np = 1.0 - p[0].z;

    camera().setPerspectiveProjection(p[1].x, p[0].x, p[0].y, p[1].y, 1.0 - p[0].z, 10000.0);
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

    if(isKeyDownEvent(evt, SDLK_EQUALS)) {
      ctr += ivec2(0, 1);
      tileMesh->rebuildMesh(2, ctr);
      std::cout << "rebuild + " << to_string(ctr) << std::endl;
    }

    if(isKeyDownEvent(evt, SDLK_MINUS)) {
      ctr -= ivec2(0, 1);
      tileMesh->rebuildMesh(2, ctr);
      std::cout << "rebuild - " << to_string(ctr) << std::endl;
    }
  }

  void onDraw(Renderer& rndr) {
    rndr.clearViewport();


    rndr.enableDepthBuffer();

    glBindFramebuffer(GL_FRAMEBUFFER, vcLookubFramebuffer);
    glViewport(0, 0, width(), height());

    rndr.setProgram(vcLookupProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileTextureArray());
    glUniform1i(glGetUniformLocation(vcLookupProgram, "texid"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileDepthTextureArray());
    glUniform1i(glGetUniformLocation(vcLookupProgram, "depthId"), 1);

    rndr.draw(tileMesh->geometry(), mat4(1.0), PrimitiveType::TRIANGLES);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    rndr.disableDepthBuffer();


    rndr.setProgram(drawSceneProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileTextureArray());
    glUniform1i(glGetUniformLocation(drawSceneProgram, "texid"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tileMesh->tileDepthTextureArray());
    glUniform1i(glGetUniformLocation(drawSceneProgram, "depthId"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, vcLookupTexture);
    glUniform1i(glGetUniformLocation(drawSceneProgram, "vcTex"), 2);

    glUniform1fv(glGetUniformLocation(drawSceneProgram, "np"), 1, &np);
    glUniform3fv(glGetUniformLocation(drawSceneProgram, "cameraPos"), 1, value_ptr(camera().getPosition()));
    glUniform2fv(glGetUniformLocation(drawSceneProgram, "viewportSize"), 1, value_ptr(vec2(width(), height())));

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
