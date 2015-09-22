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
  private:
    GLuint mNumLayers = 0;
    ivec2 mImgDims = ivec2(0);
    Mode mMode = FRONT;
  public:

    Config() {}

    Config(ivec2 imgDims, GLuint numLayers) : mNumLayers(numLayers), mImgDims(imgDims) {}

    GLuint numLayers() const {
      return mNumLayers;
    }

    Mode mode() const {
      return mMode;
    }

    ivec2 imgDims() const {
      return mImgDims;
    }

    void setMode(const Mode& mode) {
      mMode = mode;
    }

    void nextMode() {
      mMode = static_cast<Mode>((mMode + 1) % NUM_FACES);
      cout << to_string(mMode) << endl;
    }

    void prevMode() {
      mMode = static_cast<Mode>((mMode - 1) % NUM_FACES);
      cout << to_string(mMode) << endl;
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
  GLuint mRenderViewProgram = 0, mRenderFboTexProgram = 0;
  GLuint mFramebuffer = 0;
  GLuint mFramebufferTexture = 0;
  GLuint mDepthRenderbuffer;
  Geometry mFullScreenQuad;

public:
  typedef Tuple<vec2> Vertex;

  ViewRenderer(size_t numLayers, size_t overlap, GLProgramBuilder& programBuilder, const ivec2& imgDims) : mConfig(imgDims, numLayers){
    // Load textures for each view of the environment map
    mImgTexArrays = make_gl_tex2darrays<Config::NUM_FACES>(imgDims.x, imgDims.y, mConfig.numLayers(), GL_RGB16F);
    mDepthTexArrays = make_gl_tex2darrays<Config::NUM_FACES>(imgDims.x, imgDims.y, mConfig.numLayers(), GL_RGB16F);

    for(unsigned i = 0; i < Config::NUM_FACES; i++) {
      for(unsigned l = 0; l < mConfig.numLayers(); l++) {
        const string fileSuffix = /*Config::to_string(static_cast<Config::Mode>(i))*/ string("FrontWallView") +
            string("_") + to_string(l) + string("_") + to_string(overlap) + string(".ppm");
        const string imgFilename = string("kernel_images/") + fileSuffix;
        const string depthFilename = string("kernel_images/db_") + fileSuffix;

        cout << "Loading " << fileSuffix << endl;

        load_ppm_to_gl_tex2darray(imgFilename, mImgTexArrays[i], l);
        load_ppm_to_gl_tex2darray(depthFilename, mDepthTexArrays[i], l);
      }
    }

    // Set up a framebuffer so we can render output to a texture
    glGenTextures(1, &mFramebufferTexture);
    glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgDims.x, imgDims.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glGenFramebuffers(1, &mFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFramebufferTexture, 0);

    glGenRenderbuffers(1, &mDepthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, imgDims.x, imgDims.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthRenderbuffer);

    GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, drawBuffers);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw runtime_error("Error: Problem configuring framebuffer.");
    }
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Create the program used to render a single view
    mRenderViewProgram = programBuilder.buildFromFiles("shaders/composite_vert.glsl", "shaders/composite_frag.glsl");
    mRenderFboTexProgram = programBuilder.buildFromFiles("shaders/composite_vert.glsl", "shaders/texture_frag.glsl");

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

  void render(Renderer& rndr, unsigned numViews, float blendCoeff, ivec2 viewportDims, bool flip = false) {
    GLint views = numViews > mConfig.numLayers() ? mConfig.numLayers() : numViews;

    rndr.clearViewport();

    rndr.setProgram(mRenderViewProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mImgTexArrays[mConfig.mode()]);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "imageTexArray"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, mDepthTexArrays[mConfig.mode()]);
    glUniform1i(glGetUniformLocation(mRenderViewProgram, "depthTexArray"), 1);

    glUniform1iv(glGetUniformLocation(mRenderViewProgram, "numLayers"), 1, &views);

    GLint uFlip = 0;
    if(flip) {
      uFlip = 1;
    }

    glUniform1f(glGetUniformLocation(mRenderViewProgram, "kernelSize"), 6.0f);
    glUniform1f(glGetUniformLocation(mRenderViewProgram, "overlap"), 3.0f);
    glUniform1iv(glGetUniformLocation(mRenderViewProgram, "flip"), 1, &uFlip);
    glUniform1fv(glGetUniformLocation(mRenderViewProgram, "blendCoeff"), 1, &blendCoeff);
    glUniform2fv(glGetUniformLocation(mRenderViewProgram, "viewportSize"), 1, value_ptr(vec2(viewportDims)));

    rndr.draw(mFullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }

  void renderFBOTex(Renderer& rndr, ivec2 viewportDims) {
    rndr.clearViewport();

    rndr.setProgram(mRenderFboTexProgram);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
    glUniform1i(glGetUniformLocation(mRenderFboTexProgram, "tex"), 2);

    glUniform2fv(glGetUniformLocation(mRenderFboTexProgram, "viewportSize"), 1, value_ptr(vec2(viewportDims)));

    rndr.draw(mFullScreenQuad, mat4(1.0), PrimitiveType::TRIANGLES);
  }

  void renderToFile(Renderer& rndr, const string& outFilename) {
    const size_t NUM_CHANNELS = 4;
    const size_t OUT_IMG_SIZE = mConfig.imgDims().x * mConfig.imgDims().y * NUM_CHANNELS * 12;
    const size_t OUT_IMG_WIDTH = mConfig.imgDims().x * 4;
    const size_t OUT_IMG_HEIGHT = mConfig.imgDims().y * 3;

    GLubyte* outImgData = new GLubyte[OUT_IMG_SIZE];
    GLubyte* tmpImgData = new GLubyte[mConfig.imgDims().x * mConfig.imgDims().y * NUM_CHANNELS];

    memset(outImgData, 255, sizeof(GLubyte) * OUT_IMG_SIZE);

    rndr.setProgram(mRenderViewProgram);

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    glViewport(0, 0, mConfig.imgDims().x, mConfig.imgDims().y);

    for(size_t i = 0; i < Config::NUM_FACES; i++) {
      render(rndr, mConfig.numLayers(), 0.5, mConfig.imgDims(), GL_TRUE);

      glReadPixels(0, 0, mConfig.imgDims().x, mConfig.imgDims().y, GL_RGBA, GL_UNSIGNED_BYTE, tmpImgData);

      for(size_t v = 0; v < static_cast<size_t>(mConfig.imgDims().y); v++) {
        const size_t BYTES_PER_TMP_ROW = mConfig.imgDims().x * NUM_CHANNELS;
        for(size_t u = 0; u < BYTES_PER_TMP_ROW; u++) {
          const size_t BYTES_PER_OUT_ROW = OUT_IMG_WIDTH * NUM_CHANNELS;

          size_t outImgOffset = 0;
          switch(mConfig.mode()) {
          case Config::Mode::FRONT:
            outImgOffset = (BYTES_PER_OUT_ROW * mConfig.imgDims().y) + BYTES_PER_TMP_ROW;
            break;
          case Config::Mode::BACK:
            outImgOffset = (BYTES_PER_OUT_ROW * mConfig.imgDims().y) + (BYTES_PER_TMP_ROW * 3);
            break;
          case Config::Mode::LEFT:
            outImgOffset = BYTES_PER_OUT_ROW * mConfig.imgDims().y;
            break;
          case Config::Mode::RIGHT:
            outImgOffset = (BYTES_PER_OUT_ROW * mConfig.imgDims().y) + (BYTES_PER_TMP_ROW * 2);
            break;
          case Config::Mode::TOP:
            outImgOffset = BYTES_PER_TMP_ROW;
            break;
          case Config::Mode::BOTTOM:
            outImgOffset = (BYTES_PER_OUT_ROW * mConfig.imgDims().y * 2) + BYTES_PER_TMP_ROW;
            break;
          }

          outImgData[outImgOffset + (v * BYTES_PER_OUT_ROW) + u] = tmpImgData[v * BYTES_PER_TMP_ROW + u];
        }
      }

      mConfig.nextMode();
    }

    int saveImgRes = SOIL_save_image(outFilename.c_str(), SOIL_SAVE_TYPE_BMP, OUT_IMG_WIDTH, OUT_IMG_HEIGHT, 4, outImgData);

    delete[] outImgData;
    delete[] tmpImgData;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(saveImgRes == 0) {
      throw runtime_error(string("Error: Error saving image envmap.bmp: ") + string(SOIL_last_result()));
    }
  }
};

class App: public BasicGLWindow {
  GLProgramBuilder mProgramBuilder;
  unique_ptr<ViewRenderer> mViewRenderer;

  unsigned mNumViews = 1;
  float mBlendCoefficient = 0.0;
  bool mSaveEnvmapFlag = false, mSaveScreenshotFlag = false;

public:
  App(unsigned w, unsigned h) : BasicGLWindow(w, h) {}

  void onCreate(Renderer& rndr) {
    rndr.setClearColor(vec4(0.1, 0.1, 0.5, 1.0));
    rndr.enableFaceCulling();
    rndr.enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mProgramBuilder.addIncludeDir("shaders/glsl330");
    mProgramBuilder.addIncludeDir("shaders");

    mViewRenderer = make_unique<ViewRenderer>(5, 3, mProgramBuilder, ivec2(512));
  }


  void onEvent(const SDL_Event& evt) {
    if(isKeyDownEvent(evt, SDLK_UP)) {
      mNumViews = glm::min<unsigned>(mViewRenderer->config().numLayers(), mNumViews + 1);
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
      mViewRenderer->config().nextMode();
    }
    if(isKeyDownEvent(evt, SDLK_p)) {
      mViewRenderer->config().prevMode();
    }
    if(isKeyDownEvent(evt, SDLK_w)) {
      mSaveEnvmapFlag = true;
    }
    if(isKeyDownEvent(evt, SDLK_s)) {
      mSaveScreenshotFlag = true;
    }
  }

  void onDraw(Renderer& rndr) {
    if(mSaveEnvmapFlag) {
      cout << "Rendering environment map..." << endl;

      mViewRenderer->renderToFile(rndr, "envmap.bmp");
      mSaveEnvmapFlag = false;

      glViewport(0, 0, width(), height());
      cout << "Done!" << endl;

    } else if(mSaveScreenshotFlag) {
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
  App w(512, 512);
  w.mainLoop();
}
