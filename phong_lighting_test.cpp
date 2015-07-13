#include <SOIL/SOIL.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "renderer/renderer.h"
#include "renderer/camera.h"
#include "utils/sdl_gl_window.h"

using namespace glm;
using namespace std;
using namespace renderer;

struct Material {
  vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
  float shine = 0.0;
};

typedef Renderer<GlVersion::GL330> Rndr;

struct SimpleMirrorGLWindow: SDLGLWindow {
  // Renderer to render objects in the scene
  Rndr* rndr = nullptr;

  // An FPS camera
  FirstPersonCamera camera;
  const float FOV_Y = 45.0f;

  // Geometry for a cube, a plane, and a sphere
  Geometry cubeMesh;
  Geometry planeMesh;
  Geometry sphereMesh;

  // Materials for the sphere and cube
  Material sphereMaterial;
  Material cubeMaterial;
  Material planeMaterial;

  // Transformations for geometry in the scene
  mat4 cubeTransform = scale(translate(mat4(1.0), vec3(0.0, 4.0, 0.0)), vec3(4.0, 8.0, 4.0));
  mat4 sphereTransform = translate(mat4(1.0), vec3(-5.0, 1.5, -4.0));
  mat4 planeTransform = scale(rotate(mat4(1.0), glm::half_pi<float>(), vec3(1.0, 0.0, 0.0)), vec3(10000.0));

  // Programs used to render objects in the scene
  GLuint phongProgram = 0, drawNormalsProgram = 0, drawLightsProgram = 0, drawSkyboxProgram = 0;

  GLuint cubemapTex = 0;

  // Locations of uniforms in programs
  static const GLuint COLOR1_LOC = 2;
  static const GLuint COLOR2_LOC = 3;

  SimpleMirrorGLWindow(size_t w, size_t h) :
      SDLGLWindow(w, h) {
  }

  void drawNormals(const vec4& color1, const vec4& color2) {
    rndr->setProgram(drawNormalsProgram);
    glUniform4fv(COLOR1_LOC, 1, glm::value_ptr(color1));
    glUniform4fv(COLOR2_LOC, 1, glm::value_ptr(color2));
    rndr->draw(sphereMesh.normal_view_vao, sphereMesh.num_vertices * 2, sphereTransform, LINES);
    rndr->draw(cubeMesh.normal_view_vao, cubeMesh.num_vertices * 2, cubeTransform, LINES);
    rndr->draw(planeMesh.normal_view_vao, planeMesh.num_vertices * 2, planeTransform, LINES);
    for(size_t i = 0; i < rndr->numLights(); i++) {
      const mat4 lightTransform = scale(translate(mat4(1.0), rndr->lightPosition(i)), vec3(0.1));
      rndr->draw(cubeMesh.normal_view_vao, (size_t)(cubeMesh.num_vertices * 2), lightTransform, LINES);
    }
  }

  void setMaterial(const Material& material) {
    static GLuint MAT_LOC_DIFF = glGetUniformLocation(phongProgram, "material.diffuse");
    static GLuint MAT_LOC_SPEC = glGetUniformLocation(phongProgram, "material.specular");
    static GLuint MAT_LOC_SHINE = glGetUniformLocation(phongProgram, "material.shine_exp");
    glUniform4fv(MAT_LOC_DIFF, 1, value_ptr(material.diffuse));
    glUniform4fv(MAT_LOC_SPEC, 1, value_ptr(material.specular));
    glUniform1f(MAT_LOC_SHINE, material.shine);
  }

  void setup(SDLGLWindow& w) {
    // Initialize the renderer
    rndr = new Rndr();
    rndr->setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
    rndr->enableDepthBuffer();
    rndr->enableFaceCulling();

    rndr->addShaderIncludeDir("shaders");

    // Build shader programs
    phongProgram = rndr->makeProgramFromFiles("shaders/phong_vertex.glsl",
                                                  "shaders/phong_frag.glsl");

    // Make debug program which draws the normals of a piece of geometry
    drawNormalsProgram = rndr->makeProgramFromFiles("shaders/draw_normals_vert.glsl",
                                                        "shaders/draw_normals_frag.glsl");

    // Make program to draw lights
    drawLightsProgram = rndr->makeProgramFromFiles("shaders/draw_lights_vert.glsl",
                                                       "shaders/draw_lights_frag.glsl");


    // Create geometry
    cubeMesh = Geometry::make_cube(vec3(1.0), false);
    sphereMesh = Geometry::make_sphere(1.5, 100, 100);
    planeMesh = Geometry::make_plane(100, 100);

    // Setup the camera
    camera.setPosition(vec3(0.0, 1.0, -7.5));
    camera.setPerspectiveProjection(FOV_Y, w.aspectRatio(), 0.5, 1000.0);
    camera.setCameraVelocity(vec2(0.1));


    // Setup a grid of 9 lights above the center of the ball and one light along the +z axis
    Light l1 = {vec4(0.0),
                vec4(0.5, 0.5, 0.5, 1.0),
                vec4(0.325, 0.325, 0.325, 1.0)};
    vec4 center(0.0, 15.0, -5.0, 1.0);
    vec2 squareSize(42.5);
    for(int i = 0; i < 3; i++) {
      for(int j = 0; j < 3; j++) {
        size_t light_index = 3 * i + j;
        vec4 offset((i-1)*squareSize.x/2.0, 0.0, (j-1)*squareSize.y/2.0, 0.0);
        rndr->enableLight(light_index);
        rndr->setLight(light_index, l1);
        rndr->setLightPos(light_index, (center + offset));
        rndr->setLightAttenuation(light_index, 25.0);
      }
    }

    Light l2 = {vec4(0.0, 2.0, -5.0, 1.0),
                vec4(0.3525, 0.3525, 0.3525, 1.0),
                vec4(0.1, 0.1, 0.1, 1.0) };
    rndr->enableLight(9);
    rndr->setLight(9, l2);
    rndr->setLightAttenuation(9, 20.0);

    rndr->setGlobalAmbient(vec4(0.005, 0.005, 0.005, 1.0));

//    unsigned char* imgdata;
//    int width, height;
//    const char* imgFiles[] = {"+x.png", "-x.png", "+y.png", "-y.png", "+z.png", "-z.png"};
//    GLenum imgFlags[] = {
//    		GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
//    		GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
//			GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
//    glGenTextures(1, &cubemapTex);
//    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTex);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
//    for(unsigned i = 0; i < 6; i++) {
//    	imgdata = SOIL_load_image(imgFiles[i], &width, &height, 0, SOIL_LOAD_RGB);
//    	glTexImage2D(imgFlags[i], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imgdata);
//    	SOIL_free_image_data(imgdata);
//    }

    // Setup materials
    planeMaterial.diffuse = vec4(0.005, 0.005, 0.015, 1.0);
    planeMaterial.specular = vec4(0.25);
    planeMaterial.shine = 1024.0f;

    cubeMaterial.diffuse = vec4(0.015, 0.01, 0.01, 1.0);
    cubeMaterial.specular = vec4(0.17, 0.17, 0.17, 1.0);
    cubeMaterial.shine = 10.0f;

    sphereMaterial.diffuse = vec4(0.0, 0.05, 0.15, 1.0);
    sphereMaterial.specular = vec4(0.0, 0.0, 0.0, 1.0);
    sphereMaterial.shine = 2048.0f;


    // Move mouse cursor to the middle of the screen and hide it
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
      if(event.key.keysym.sym == SDLK_f) {
    	  setFullScreen(true);
      }
      if(event.key.keysym.sym == SDLK_ESCAPE) {
    	  setFullScreen(false);
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

    // Draw the and the cube sphere
    rndr->setProgram(phongProgram);

    setMaterial(planeMaterial);
    rndr->draw(planeMesh, planeTransform);

    setMaterial(cubeMaterial);
    rndr->draw(cubeMesh, cubeTransform);

    setMaterial(sphereMaterial);
    rndr->draw(sphereMesh, sphereTransform);

    // Draw a cube over each light
    rndr->setProgram(drawLightsProgram);
    for(size_t i = 0; i < rndr->numLights(); i++) {
      glUniform1ui(1, i);
      rndr->draw(cubeMesh, scale(translate(mat4(1.0), rndr->lightPosition(i)), vec3(0.1)));
    }
  }

  void teardown(SDLGLWindow& w) {
    delete rndr;

    glDeleteBuffers(1, &sphereMesh.vbo);
    glDeleteBuffers(1, &sphereMesh.ibo);
    glDeleteBuffers(1, &sphereMesh.normal_view_vbo);
    glDeleteVertexArrays(1, &sphereMesh.vao);
    glDeleteVertexArrays(1, &sphereMesh.normal_view_vao);

    glDeleteBuffers(1, &cubeMesh.vbo);
    glDeleteBuffers(1, &cubeMesh.ibo);
    glDeleteBuffers(1, &cubeMesh.normal_view_vbo);
    glDeleteVertexArrays(1, &cubeMesh.vao);
    glDeleteVertexArrays(1, &cubeMesh.normal_view_vao);

    glDeleteProgram(phongProgram);
    glDeleteProgram(drawLightsProgram);
    glDeleteProgram(drawNormalsProgram);
    glDeleteProgram(drawSkyboxProgram);
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(640, 480);
  w.mainLoop();
}
