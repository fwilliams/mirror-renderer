#include "composite.h"

#include <glm/gtc/matrix_transform.hpp>

#include "geometry/3d_primitives.h"
#include "geometry/vertex.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

class ViewRenderer {
  GLuint viewTexArray = 0;
  GLuint renderViewProgram = 0;

  Geometry fullScreenQuad;
  vec2 viewportDims;

  unsigned numLayers;

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t viewportX, size_t viewportY, GLProgramBuilder& programBuilder) : numLayers(numLayers), viewportDims(vec2(viewportX, viewportY)){

    // A texture array to hold transformed views
    const ivec2 dims(800, 600);
    viewTexArray = RenderMesh::makeTextureArray(dims.x, dims.y, numLayers);
    for(unsigned i = 1; i <= numLayers; i++) {
      string filename = string("4_wall_mirror_dup_") + to_string(i) + string(".png");
      RenderMesh::loadImgToTexArray(filename, viewTexArray, i-1);

    }

    // Create the program used to render a single view
    renderViewProgram = programBuilder.buildFromFiles("shaders/composite_vert.glsl", "shaders/composite_frag.glsl");

    // Construct a full screen quad
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

  void render(Renderer& rndr, unsigned numViews) {
    int views = numViews > numLayers ? numLayers : numViews;

    rndr.clearViewport();

    rndr.setProgram(renderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, viewTexArray);
    glUniform1i(glGetUniformLocation(renderViewProgram, "imageTexArray"), 0);

    glUniform1iv(glGetUniformLocation(renderViewProgram, "numLayers"), 1, &views);

    glUniform2fv(glGetUniformLocation(renderViewProgram, "viewportSize"), 1, value_ptr(viewportDims));
    rndr.draw(fullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }
};

class App: public InteractiveGLWindow {
  GLProgramBuilder programBuilder;

  unique_ptr<ViewRenderer> viewRenderer;

  unsigned numViews = 1;

public:
  App(unsigned w, unsigned h) : InteractiveGLWindow(w, h) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();
    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    programBuilder.addIncludeDir("shaders/glsl330");
    programBuilder.addIncludeDir("shaders");

    viewRenderer = make_unique<ViewRenderer>(5, width(), height(), programBuilder);
  }

  void onUpdate() {
  }

  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_UP)) {
      numViews += 1;
    }
    if(isKeyDownEvent(evt, SDLK_DOWN)) {
      numViews = numViews == 0 ? 0 : numViews - 1;

    }
  }

  void onDraw(Renderer& rndr) {
    viewRenderer->render(rndr, numViews);
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
