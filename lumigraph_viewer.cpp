#include <string>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include "utils/gl_utils.h"
#include "utils/interactive_window.h"

#include "geometry/3d_primitives.h"
#include "geometry/tile_mesh.h"
#include "geometry/vertex.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;
using namespace boost::filesystem;

using RenderMesh = geometry::TileMesh<geometry::QuadPlanarTileSet>;

class ViewRenderer {
  GLuint lumigraphTex = 0;
  GLuint renderViewProgram = 0;

  Geometry fullScreenQuad;
  vec2 viewportDims;

  unsigned numLayers;

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t viewportX, size_t viewportY, GLProgramBuilder& programBuilder) :
    viewportDims(vec2(viewportX, viewportY)), numLayers(numLayers) {

    lumigraphTex = RenderMesh::makeTextureArray(512, 512, 64*64);

    for(auto entry : boost::make_iterator_range(directory_iterator(path("lumigraph")))) {
      string path = entry.path().filename().c_str();
      if(boost::starts_with(path, "neg_z_")) {
        string rest = path.substr(sizeof("neg_z_")-1);
        string i_str = rest.substr(0, rest.find("_"));
        string j_str = rest.substr(i_str.length()+1, rest.find(".") - i_str.length()-1);
        unsigned i = stoi(i_str), j = stoi(j_str);

        cout << "Loading texture -Z: (" << i << ", " << j << ")" << endl;
        RenderMesh::loadImgToTexArray(string("lumigraph/") + string(path.c_str()), lumigraphTex, i*64 + j);
      }
    }

    // A texture array to hold transformed views
    const ivec2 dims(800, 600);


    // Create the program used to render a single view
    renderViewProgram = programBuilder.buildFromFiles("shaders/composite_vert.glsl", "shaders/lumigraph_vert.glsl");

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

  void render(Renderer& rndr, unsigned numViews) {
    rndr.clearViewport();
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
    App w(10, 10);
    w.mainLoop();
  }
}
