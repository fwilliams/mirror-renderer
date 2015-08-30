#include "virtual_copy_transform.h"

#include <glm/gtc/matrix_transform.hpp>

#include "geometry/3d_primitives.h"
#include "geometry/vertex.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

class ViewRenderer {
  GLuint viewTexArray = 0;
  GLuint renderFramebuffer = 0;
  GLuint renderViewProgram = 0;

  unique_ptr<RenderMesh> renderMesh;
  Geometry fullScreenQuad;
  vec2 viewportDims;

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t radius, size_t viewportX, size_t viewportY, GLProgramBuilder& programBuilder) : viewportDims(vec2(viewportX, viewportY)){
    // Data structure holding information about views
    renderMesh = make_unique<RenderMesh>(radius);
    Geometry g = renderMesh->geometry();

    // A texture array to hold transformed views
    const ivec2 dims = renderMesh->getViewImageDims();
    viewTexArray = RenderMesh::makeTextureArray(dims.x, dims.y, renderMesh->numTextures());

    // Create the program used to render a single view
    renderViewProgram = programBuilder.buildFromFiles("shaders/tile_view_vert.glsl", "shaders/tile_view_frag.glsl");

    // Build a framebuffer to render views to textures
    glGenFramebuffers(1, &renderFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
    glViewport(0, 0, viewportX, viewportY);

    GLuint depthrenderbuffer;
    glGenRenderbuffers(1, &depthrenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, viewportX, viewportY);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

    GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);
    assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // All views are rendered to a fullscreen quad
    // Construct such a quad
    fullScreenQuad = Geometry::makeGeometry<Vertex>(4, 6);
    glBindBuffer(GL_ARRAY_BUFFER, fullScreenQuad.vbo);
    Vertex* verts = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fullScreenQuad.ibo);
    GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));
    verts[0] = Vertex{vec2(-1, -1)};
    verts[1] = Vertex{vec2(1, -1)};
    verts[2] = Vertex{vec2(1, 1)};
    verts[3] = Vertex{vec2(-1, 1)};
    inds[0] = 0; inds[1] = 1; inds[2] = 2; inds[3] = 0; inds[4] = 2; inds[5] = 3;

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  GLuint viewTextureArray() {
    return viewTexArray;
  }

  void render(Renderer& rndr, const vec3& view, const vec3& cameraPos) {
//    glBindFramebuffer(GL_FRAMEBUFFER, renderFramebuffer);
    // TODO: Query the texture for the specific view
//    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_ARRAY, viewTexArray, renderMesh->indexForView(view));

//    cout << to_string(view) << "->" << renderMesh->indexForView(view) << endl;


    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.clearViewport();

    rndr.setProgram(renderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderMesh->tileTextureArray());
    glUniform1i(glGetUniformLocation(renderViewProgram, "imageTexArray"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, renderMesh->tileDepthTextureArray());
    glUniform1i(glGetUniformLocation(renderViewProgram, "depthTexArray"), 1);

    const GLuint texindex = renderMesh->indexForView(view);
    const vec3 v = vec3(view.x, 0.0, view.y)  + vec3(RenderMesh::edgeFaces()[view.z].center());
    const GLfloat f = v.z;

    glUniform1uiv(glGetUniformLocation(renderViewProgram, "texindex"), 1, &texindex);
    glUniform1fv(glGetUniformLocation(renderViewProgram, "f"), 1, &f);
    glUniform3fv(glGetUniformLocation(renderViewProgram, "cameraPos"), 1, value_ptr(cameraPos));
    glUniform2fv(glGetUniformLocation(renderViewProgram, "viewportSize"), 1, value_ptr(viewportDims));

    rndr.draw(fullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }
};

class App: public InteractiveGLWindow {
  GLProgramBuilder programBuilder;

  unique_ptr<ViewRenderer> viewRenderer;

  Config config;

public:
  App(unsigned w, unsigned h) : InteractiveGLWindow(w, h) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();
    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    programBuilder.addIncludeDir("shaders/glsl330");
    programBuilder.addIncludeDir("shaders");

    viewRenderer = make_unique<ViewRenderer>(2, width(), height(), programBuilder);
  }

  void onUpdate() {
  }

  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_n)) {
      config.nextMirror();
    }
    if(isKeyDownEvent(evt, SDLK_x)) {
      config.upView();
    }
    if(isKeyDownEvent(evt, SDLK_c)) {
      config.downView();
    }
  }

  void onDraw(Renderer& rndr) {
    viewRenderer->render(rndr, config.currentView(), camera().getPosition());
  }
};

int main(int argc, char** argv) {
  if(argc > 2 && strcmp(argv[1], "-p") == 0) {
    size_t radius = atoi(argv[2]);
    RenderMesh tileMesh(radius);
    tileMesh.printTextureNames();
  } else {
    App w(1280, 1024);
    w.mainLoop();
  }
}
