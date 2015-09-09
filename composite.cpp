#include <glm/gtc/matrix_transform.hpp>

#include "utils/gl_utils.h"
#include "utils/basic_window.h"

#include "geometry/3d_primitives.h"
#include "geometry/vertex.h"

#include "texture.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace utils;

class ViewRenderer {
  struct Config {
    const static size_t NUM_FACES = 6;
    enum Mode { FRONT, BACK, LEFT, RIGHT, TOP, BOTTOM };
    GLuint numLayers;

    Mode mode;

    void next_mode() {
      mode = static_cast<Mode>((mode + 1) % NUM_FACES);
      cout << to_string(mode) << endl;
    }

    void prev_mode() {
      mode = static_cast<Mode>((mode - 1) % NUM_FACES);
      cout << to_string(mode) << endl;
    }

    static string to_string(const Mode& mode) {
      switch(mode) {
      case FRONT:
        return "FrontWallView";
      case BACK:
        return "BackWallView";
      case LEFT:
        return "LeftWallView";
      case RIGHT:
        return "RightWallView";
      case TOP:
        return "TopWallView";
      case BOTTOM:
        return "BottomWallView";
      default:
        throw runtime_error("Error: Invalid mode passed to Config::to_string(mode).");
      }
    }
  } mConfig;

  array<GLuint, Config::NUM_FACES> mImgTexArrays, mDepthTexArrays;
  GLuint mRenderViewProgram = 0;
  Geometry mFullScreenQuad;


public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t overlap, GLProgramBuilder& programBuilder) {
    mConfig.numLayers = numLayers;
    mConfig.mode = Config::Mode::FRONT;

    const ivec2 dims(256, 256);

    // Load textures for each view of the environment map
    mImgTexArrays = make_gl_tex2darrays<Config::NUM_FACES>(dims.x, dims.y, mConfig.numLayers, GL_RGB16F);
    mDepthTexArrays = make_gl_tex2darrays<Config::NUM_FACES>(dims.x, dims.y, mConfig.numLayers, GL_RGB16F);

    for(unsigned i = 0; i < Config::NUM_FACES; i++) {
      for(unsigned l = 0; l < mConfig.numLayers; l++) {
        const string fileSuffix = Config::to_string(static_cast<Config::Mode>(i)) +
            string("_") + to_string(l) + string("_") + to_string(overlap) + string(".ppm");
        const string imgFilename = string("envmaps/") + fileSuffix;
        const string depthFilename = string("envmaps/db_") + fileSuffix;

        cout << "Loading " << fileSuffix << endl;

        load_ppm_to_gl_tex2darray(imgFilename, mImgTexArrays[i], l);
        load_ppm_to_gl_tex2darray(depthFilename, mDepthTexArrays[i], l);
      }
    }

    // Create the program used to render a single view
    mRenderViewProgram = programBuilder.buildFromFiles("shaders/composite_vert.glsl", "shaders/composite_frag.glsl");

    // Construct a full screen quad
    mFullScreenQuad = Geometry::makeGeometry<Vertex>(4, 6);
    glBindBuffer(GL_ARRAY_BUFFER, mFullScreenQuad.vbo);
    Vertex* verts = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mFullScreenQuad.ibo);
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

  Config& config() {
    return mConfig;
  }

  void render(Renderer& rndr, unsigned numViews, float blendCoeff, ivec2 viewportDims) {
    GLint views = numViews > mConfig.numLayers ? mConfig.numLayers : numViews;

    rndr.clearViewport();

    rndr.setProgram(mRenderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mImgTexArrays[mConfig.mode]);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "imageTexArray"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexArrays[mConfig.mode]);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "depthTexArray"), 1);

    glUniform1iv(glGetUniformLocation(mRenderViewProgram, "numLayers"), 1, &views);
    glUniform1fv(glGetUniformLocation(mRenderViewProgram, "blendCoeff"), 1, &blendCoeff);
    glUniform2fv(glGetUniformLocation(mRenderViewProgram, "viewportSize"), 1, value_ptr(vec2(viewportDims)));

    rndr.draw(mFullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }
};

class App: public BasicGLWindow {
  GLProgramBuilder mProgramBuilder;
  unique_ptr<ViewRenderer> mViewRenderer;

  unsigned mNumViews = 1;
  float mBlendCoefficient = 0.0;

public:
  App(unsigned w, unsigned h) : BasicGLWindow(w, h) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();
    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mProgramBuilder.addIncludeDir("shaders/glsl330");
    mProgramBuilder.addIncludeDir("shaders");

    mViewRenderer = make_unique<ViewRenderer>(2, 0, mProgramBuilder);
  }

  void onUpdate() {
  }

  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_UP)) {
      mNumViews = glm::min<unsigned>(mViewRenderer->config().numLayers, mNumViews + 1);
    }
    if(isKeyDownEvent(evt, SDLK_DOWN)) {
      mNumViews = glm::max<unsigned>(0, mNumViews - 1);
    }
    if(isKeyDownEvent(evt, SDLK_LEFT)) {
      mBlendCoefficient = glm::max<float>(0.0f, mBlendCoefficient - 0.01f);
      cout << "blend coefficient: " << mBlendCoefficient << endl;
    }
    if(isKeyDownEvent(evt, SDLK_RIGHT)) {
      mBlendCoefficient = glm::min<float>(1.0f, mBlendCoefficient + 0.01f);
      cout << "blend coefficient: " << mBlendCoefficient << endl;
    }
    if(isKeyDownEvent(evt, SDLK_n)) {
      mViewRenderer->config().next_mode();
    }
    if(isKeyDownEvent(evt, SDLK_p)) {
      mViewRenderer->config().prev_mode();
    }
  }

  void onDraw(Renderer& rndr) {
    mViewRenderer->render(rndr, mNumViews, mBlendCoefficient, ivec2(width(), height()));
  }
};

int main(int argc, char** argv) {
  App w(800, 600);
  w.mainLoop();
}
