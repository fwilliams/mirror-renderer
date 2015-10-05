#include <functional>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string/predicate.hpp>
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

namespace fs = boost::filesystem;

class ViewRenderer {
  class Config {
    string mImagePath;
    ivec2 mImgDims = ivec2(0);
    size_t mNumImages = 0;
  public:

    Config() {}

    Config(ivec2 imgDims, const string& imagedirname) : mImagePath(imagedirname), mImgDims(imgDims) {
      using namespace std::placeholders;

      mNumImages = std::count_if(fs::directory_iterator(imagedirname),
                                 fs::directory_iterator(),
                                 bind(static_cast<bool(*)(const fs::path&)>(fs::is_regular_file),
                                      bind( &fs::directory_entry::path, _1 )));
      mNumImages -= 1;
      mNumImages /= 2;
    }

    ivec2 imgDims() const {
      return mImgDims;
    }

    size_t numImages() const {
      return mNumImages;
    }

    const string& imgPath() const {
      return mImagePath;
    }

  } mConfig;

  GLuint mImgTexArray, mDepthTexArray, mBackgroundTex;
  GLuint mRenderViewProgram = 0;
  Geometry mFullScreenQuad;

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t overlap, GLProgramBuilder& programBuilder, const ivec2& imgDims, const string& imgDir) :
    mConfig(imgDims, imgDir) {

    using namespace fs;

    mImgTexArray = make_gl_tex2darray(imgDims.x, imgDims.y, mConfig.numImages(), GL_RGB16F);
    mDepthTexArray = make_gl_tex2darray(imgDims.x, imgDims.y, mConfig.numImages(), GL_RGB16F);

    { // Load all the textures in the image directory
      unsigned lc = 0;
      for(auto entry : boost::make_iterator_range(directory_iterator(path(mConfig.imgPath())))) {
        const string filename = entry.path().filename().string();
        if(!boost::starts_with(filename, "db_") && !boost::starts_with(filename, "background")) {
          cout << "Loading Image: " << filename << endl;
          load_ppm_to_gl_tex2darray(entry.path().string(), mImgTexArray, lc);
          load_ppm_to_gl_tex2darray(mConfig.imgPath() + string("/db_") + filename, mDepthTexArray, lc++);
        }
      }

      glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
      glGenTextures(1, &mBackgroundTex);
      glBindTexture(GL_TEXTURE_2D, mBackgroundTex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      size_t width, height, bytesPerChannel;
      uint8_t* backgroundImg = load_ppm(mConfig.imgPath() + string("/background.ppm"), width, height, bytesPerChannel);
      cout << "Bytes per channel = " << bytesPerChannel << endl;
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT, backgroundImg);
      glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
      delete backgroundImg;
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
    GLint views = numViews > mConfig.numImages() ? mConfig.numImages() : numViews;

    rndr.clearViewport();

    rndr.setProgram(mRenderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mImgTexArray);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "imageTexArray"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexArray);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "depthTexArray"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mBackgroundTex);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "backgroundTex"), 2);

    glUniform1iv(glGetUniformLocation(mRenderViewProgram, "numLayers"), 1, &views);

    glUniform1f(glGetUniformLocation(mRenderViewProgram, "kernelSize"), 6.0f);
    glUniform1f(glGetUniformLocation(mRenderViewProgram, "overlap"), 3.0f);
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
  bool mSaveScreenshotFlag = false;
  string mViewDir = "";

public:
  App(unsigned w, unsigned h, const string& viewdir) : BasicGLWindow(w, h), mViewDir(viewdir) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();
    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mProgramBuilder.addIncludeDir("shaders/glsl330");
    mProgramBuilder.addIncludeDir("shaders");

    mViewRenderer = make_unique<ViewRenderer>(5, 3, mProgramBuilder, ivec2(800, 600), mViewDir);
  }


  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_UP)) {
      mNumViews = glm::min<unsigned>(mViewRenderer->config().numImages(), mNumViews + 1);
      cout << "num views: " << mNumViews << endl;
    }
    if(isKeyDownEvent(evt, SDLK_DOWN)) {
      mNumViews = glm::min<unsigned>(mViewRenderer->config().numImages(), mNumViews - 1);
      cout << "num views: " << mNumViews << endl;
    }
    if(isKeyDownEvent(evt, SDLK_LEFT)) {
      mBlendCoefficient = glm::max<float>(0.0f, mBlendCoefficient - 0.01f);
      cout << "blend coefficient: " << mBlendCoefficient << endl;
    }
    if(isKeyDownEvent(evt, SDLK_RIGHT)) {
      mBlendCoefficient = glm::min<float>(1.0f, mBlendCoefficient + 0.01f);
      cout << "blend coefficient: " << mBlendCoefficient << endl;
    }
    if(isKeyDownEvent(evt, SDLK_s)) {
      mSaveScreenshotFlag = true;
    }
  }

  void onDraw(Renderer& rndr) {
    if(mSaveScreenshotFlag) {
      mViewRenderer->render(rndr, mNumViews, mBlendCoefficient, ivec2(width(), height()));
      if(SOIL_save_screenshot("screenshot.bmp", SOIL_SAVE_TYPE_BMP, 0, 0, width(), height()) == 0) {
        throw runtime_error(string("Error: Problem saving screenshot, screenshot.bmp: ") + string(SOIL_last_result()));
      }
      mSaveScreenshotFlag = false;
    } else {
      mViewRenderer->render(rndr, mNumViews, mBlendCoefficient, ivec2(width(), height()));
    }
  }
};

int main(int argc, char** argv) {
  string dirname = "images";
  if(argc >= 2) {
    dirname = string(argv[1]);
  }
  App w(800, 600, dirname);
  w.mainLoop();
}
