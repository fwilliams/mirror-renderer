#include <iostream>
#include <glm/glm.hpp>

#include "renderer/renderer.h"
#include "utils/gl_utils.h"
#include "utils/basic_window.h"

using namespace std;
using namespace glm;
using namespace utils;

typedef Renderer Rndr;

class ParticleExampleGLWindow: public BasicGLWindow {
  const unsigned NUM_PARTICLES = 1024 * 1024;
  const unsigned WORK_GROUP_SIZE = 128;
  const unsigned NUM_WORK_GROUPS = (NUM_PARTICLES / WORK_GROUP_SIZE);

  // Shader storage buffers for position, velocity and color
  GLuint posSSbo = 0;
  GLuint velSSbo = 0;
  GLuint colSSbo = 0;

  // Compute program object
  GLuint particlePo = 0;

  GLProgramBuilder programBuilder;

public:
  ParticleExampleGLWindow(unsigned w, unsigned h) : BasicGLWindow(w, h) {}
  void onCreate(Renderer& rndr) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);


    particlePo = programBuilder.buildComputeProgramFromFile("shaders/simple_particles.glsl");

    glGenBuffers(1, &posSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(vec4), nullptr, GL_STATIC_DRAW);

    GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT; // the invalidate makes a big difference when re-writing
    vec4* points = (vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(vec4),
        bufMask);

    srand(static_cast<unsigned>(time(0)));
    for (unsigned i = 0; i < NUM_PARTICLES; i++) {
      points[i].x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      points[i].y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      points[i].z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      points[i].w = 1.0f;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &colSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, colSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(vec4), nullptr, GL_STATIC_DRAW);

    vec4* colors = (vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
        NUM_PARTICLES * sizeof(vec4), bufMask);

    for (unsigned i = 0; i < NUM_PARTICLES; i++) {
      colors[i].r = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      colors[i].g = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      colors[i].b = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      colors[i].a = 1.0f;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    glGenBuffers(1, &velSSbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, velSSbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_PARTICLES * sizeof(vec4), nullptr, GL_STATIC_DRAW);

    vec4* velocities = (vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
        NUM_PARTICLES * sizeof(vec4), bufMask);

    for (unsigned i = 0; i < NUM_PARTICLES; i++) {
      velocities[i].x = (static_cast<float>(rand()) - static_cast<float>(RAND_MAX / 2))
          / static_cast<float>(RAND_MAX);
      velocities[i].y = (static_cast<float>(rand()) - static_cast<float>(RAND_MAX / 2))
          / static_cast<float>(RAND_MAX);
      velocities[i].z = (static_cast<float>(rand()) - static_cast<float>(RAND_MAX / 2))
          / static_cast<float>(RAND_MAX);
      velocities[i].w = 0.0f;
    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  }

  void draw(SDLGLWindow& w) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, posSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, velSSbo);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, colSSbo);

    glUseProgram(particlePo);
    glDispatchComputeGroupSizeARB(NUM_WORK_GROUPS, 1, 1, WORK_GROUP_SIZE, 1, 1);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, posSSbo);
    vec4 pt = *((vec4*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(vec4), GL_MAP_READ_BIT));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    cout << "(" << pt.x << ", " << pt.y << ", " << pt.z << ")" << endl;
  }
};

int main(int argc, char** argv) {
  ParticleExampleGLWindow w(1024, 1024);
  w.mainLoop();
}

