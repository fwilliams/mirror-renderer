#include <glm/gtc/matrix_transform.hpp>

#include "utils/gl_utils.h"
#include "utils/basic_window.h"

#include "geometry/3d_primitives.h"
#include "geometry/tile_mesh.h"
#include "geometry/vertex.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

using RenderMesh = geometry::TileMesh<geometry::QuadPlanarTileSet>;


unsigned char* ppmRead(const char* filename, int* width, int* height, size_t* channelSize) {
  FILE* fp;
  int i, w, h, d;
  unsigned char* image;
  char head[70];   // max line <= 70 in PPM (per spec).

  fp = fopen(filename, "rb");
  if(!fp) {
    perror(filename);
    return NULL;
  }

  // Grab first two chars of the file and make sure that it has the
  // correct magic cookie for a raw PPM file.
  fgets(head, 70, fp);
  if(strncmp(head, "P6", 2)) {
    throw runtime_error(string(filename) + string(": Not a raw PPM file\n"));
  }

  // Grab the three elements in the header (width, height, maxval).
  i = 0;
  while( i < 3 ) {
    fgets( head, 70, fp );
    if ( head[0] == '#' ) {// skip comments.
      continue;
    }
    if ( i == 0 ) {
      i += sscanf( head, "%d %d %d", &w, &h, &d );
    } else if ( i == 1 ) {
      i += sscanf( head, "%d %d", &h, &d );
    } else if ( i == 2 ) {
      i += sscanf( head, "%d", &d );
    }
  }

  size_t sizeofChannel = 0;
  if(d < 256) {
    sizeofChannel = sizeof(uint8_t);
  } else if(d < 65536) {
    sizeofChannel = sizeof(uint16_t);
  } else if(d < 4294967296) {
    sizeofChannel = sizeof(uint32_t);
  } else {
    throw runtime_error(string("Error: PPM channel depth, ") + to_string(d) + string(", requires greater than 32 bits of space."));
  }

  // Grab all the image data in one fell swoop.
  image = (unsigned char*) malloc( sizeofChannel * w * h * 3 );
  fread( image, sizeof( unsigned char ), sizeofChannel * w * h * 3, fp );
  fclose( fp );

  *width = w;
  *height = h;
  *channelSize = sizeofChannel;
  return image;
}

class ViewRenderer {
  GLuint viewTexArray = 0, depthTexArray = 0;
  GLuint renderViewProgram = 0;

  Geometry fullScreenQuad;
  vec2 viewportDims;

  unsigned numLayers;

  static void loadImgToTexArray(const std::string& key, GLuint tex, size_t arrayIndex) {
    // Load the image into memory
    int w, h;
    size_t channelSize;
    unsigned char* img = ppmRead(key.c_str(), &w, &h, &channelSize);

    GLenum type;
    switch(channelSize) {
    case sizeof(uint8_t):
      type = GL_UNSIGNED_BYTE;
      break;
    case sizeof(uint16_t):
      type = GL_UNSIGNED_SHORT;
      break;
    case sizeof(uint32_t):
      type = GL_UNSIGNED_INT;
      break;
    default:
      throw(std::runtime_error(std::string("Error: Invalid image channel size. Expecting, 8, 16, or 32 bits, got ") + to_string(channelSize * 8)));
    }

    cout << "Loading ppm RGB texture of size " << w << " by " << h << " and depth of " << channelSize << " bytes." << endl;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    check_gl_error();
    glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
        0, // Mipmap level
        0, 0, arrayIndex, // x-offset, y-offset, z-offset
        w, h, 1, // width, height, depth
        GL_RGB, type, img);
    check_gl_error();
    free(img);
  }

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t viewportX, size_t viewportY, GLProgramBuilder& programBuilder) :
    viewportDims(vec2(viewportX, viewportY)), numLayers(numLayers) {
    check_gl_error();

    const ivec2 dims(800, 600);

    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
    check_gl_error();
    viewTexArray = RenderMesh::makeTextureArray(dims.x, dims.y, numLayers, GL_RGB16F);
    check_gl_error();
    for(unsigned i = 0; i < numLayers; i++) {
      string filename = string("kernel_images/BackWallView_") + to_string(i) + string("_3.ppm");
      loadImgToTexArray(filename, viewTexArray, i);
      check_gl_error();
    }

    depthTexArray = RenderMesh::makeTextureArray(dims.x, dims.y, numLayers, GL_RGB16F);
    for(unsigned i = 0; i < numLayers; i++) {
      string filename = string("kernel_images/db_BackWallView_") + to_string(i) + string("_3.ppm");
      loadImgToTexArray(filename, depthTexArray, i);
      check_gl_error();
    }
    glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
    check_gl_error();


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

  void render(Renderer& rndr, unsigned numViews) {
    int views = numViews > numLayers ? numLayers : numViews;

    rndr.clearViewport();

    rndr.setProgram(renderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, viewTexArray);
    glUniform1i(glGetUniformLocation(renderViewProgram, "imageTexArray"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, depthTexArray);
    glUniform1i(glGetUniformLocation(renderViewProgram, "depthTexArray"), 1);

    glUniform1iv(glGetUniformLocation(renderViewProgram, "numLayers"), 1, &views);

    glUniform2fv(glGetUniformLocation(renderViewProgram, "viewportSize"), 1, value_ptr(viewportDims));
    rndr.draw(fullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }
};

class App: public BasicGLWindow {
  GLProgramBuilder programBuilder;

  unique_ptr<ViewRenderer> viewRenderer;

  unsigned numViews = 1;

public:
  App(unsigned w, unsigned h) : BasicGLWindow(w, h) {}

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
