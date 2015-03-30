#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "utils/sdl_gl_window.h"
#include "utils/gl_program_builder.h"
#include "utils/gl_utils.h"
#include "utils/geometry.h"

using namespace glm;
using namespace std;

struct Light {
  vec4 pos;
  vec4 diffuse;
  vec4 specular;
};

struct Material {
  vec4 diffuse = vec4(0.0, 0.0, 0.0, 1.0);
  vec4 specular = vec4(0.0, 0.0, 0.0, 1.0);
  float shine = 0.0;
};

struct Matrices {
  mat4 modelview = mat4(1.0);
  mat4 projection = mat4(1.0);
  mat4 normal = mat4(1.0); // TODO: Make this a mat3 maybe?
};

struct SimpleMirrorGLWindow: SDLGLWindow {
  ArrayOfStructs sphere_mesh;
  GLuint phongProgram = 0;
  GLuint drawNormalsProgram = 0;
  GLuint vao1 = 0, vao2 = 0;

  GLuint matricesubo = 0;
  GLuint lightsubo = 0;

  vec4 ambientColor;

  Material sphere_material;

  const GLuint MATS_UBO_BINDING_POINT = 1;
  const GLuint LIGHTS_UBO_BINDING_POINT = 2;
  const GLuint MATERIAL_LOC = 3;
  const GLuint AMBIENT_LOC = 6;
  const GLuint LIGHTS_LOC = 7;
  const GLuint COLOR1_LOC = 2;
  const GLuint COLOR2_LOC = 3;

  const GLuint NUM_LIGHTS = 10;

  SimpleMirrorGLWindow(size_t w, size_t h) :
      SDLGLWindow(w, h) {
  }

  void setup(SDLGLWindow& w) {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1, 0.1, 0.2, 1.0);
    check_gl_error()

    // Build shader programs
    drawNormalsProgram = ProgramBuilder::buildFromFiles("shaders/draw_normals_vert.glsl",
                                                        "shaders/draw_normals_frag.glsl");
    phongProgram = ProgramBuilder::buildFromFiles("shaders/phong_vertex.glsl",
                                                  "shaders/phong_frag.glsl");

    // Create sphere geometry
    sphere_mesh = make_sphere(1.5, 55, 55);

    // Setup vertex array for mesh
    glBindBuffer(GL_ARRAY_BUFFER, sphere_mesh.vbo);
    glGenVertexArrays(1, &vao1);
    glBindVertexArray(vao1);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) sizeof(vec4));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
        (void*) (sizeof(vec4) + sizeof(vec3)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup vertex array for drawing normals
    glBindBuffer(GL_ARRAY_BUFFER, sphere_mesh.normal_view_vbo);
    glGenVertexArrays(1, &vao2);
    glBindVertexArray(vao2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Setup view and projection transformations
    glGenBuffers(1, &matricesubo);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, MATS_UBO_BINDING_POINT, matricesubo);
    Matrices* mats = reinterpret_cast<Matrices*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));
    mats->projection = ortho(-2.5, 2.5, -2.5, 2.5, 0.1, 100.0);
    mats->modelview = lookAt(vec3(0.0, 22.0, 3.5), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    mats->normal = transpose(inverse(mat4(mat3(mats->modelview))));

    // Setup lights
    glGenBuffers(1, &lightsubo);
    glBindBuffer(GL_UNIFORM_BUFFER, lightsubo);
    glBufferData(GL_UNIFORM_BUFFER, NUM_LIGHTS * sizeof(Light), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, LIGHTS_UBO_BINDING_POINT, lightsubo);
    Light* lights = reinterpret_cast<Light*>(glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE));

    memset(lights, 0, NUM_LIGHTS * sizeof(Light));
    lights[0].pos = mats->modelview * vec4(15.0, 55.0, -15.0, 1.0);
    lights[0].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[0].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[1].pos = mats->modelview * vec4(-15.0, 55.0, -15.0, 1.0);
    lights[1].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[1].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[2].pos = mats->modelview * vec4(0.0, 55.0, -15.0, 1.0);
    lights[2].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[2].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[3].pos = mats->modelview * vec4(-15.0, 55.0, 0.0, 1.0);
    lights[3].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[3].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[4].pos = mats->modelview * vec4(15.0, 55.0, 0.0, 1.0);
    lights[4].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[4].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[5].pos = mats->modelview * vec4(0.0, 55.0, 0.0, 1.0);
    lights[5].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[5].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[6].pos = mats->modelview * vec4(-15.0, 55.0, 15.0, 1.0);
    lights[6].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[6].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[7].pos = mats->modelview * vec4(15.0, 55.0, 15.0, 1.0);
    lights[7].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[7].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[8].pos = mats->modelview * vec4(0.0, 55.0, 15.0, 1.0);
    lights[8].diffuse = vec4(0.15, 0.15, 0.15, 1.0);
    lights[8].specular = vec4(0.75, 0.75, 0.75, 1.0);

    lights[9].pos = mats->modelview * vec4(0.0, 0.0, 3.5, 1.0);
    lights[9].diffuse = vec4(0.55, 0.55, 0.55, 1.0);
    lights[9].specular = vec4(0.1, 0.1, 0.1, 1.0);

    // Set uniforms for lighting program
    glUseProgram(phongProgram);

    // Setup lights
    ambientColor = vec4(0.0, 0.05, 0.1, 1.0);
    glUniform4fv(AMBIENT_LOC, 1, value_ptr(ambientColor));

    // Setup material
    sphere_material.diffuse = vec4(0.0, 0.6, 0.7, 1.0);
    glUniform4fv(MATERIAL_LOC, 1, value_ptr(sphere_material.diffuse));

    sphere_material.specular = vec4(1.0, 0.4, 0.3, 1.0);
    glUniform4fv(MATERIAL_LOC + 1, 1, value_ptr(sphere_material.specular));

    sphere_material.shine = 500.0f;
    glUniform1f(MATERIAL_LOC + 2, sphere_material.shine);

    // Setup uniforms for drawing normals
    glUseProgram(drawNormalsProgram);
    glUniform4fv(COLOR1_LOC, 1, value_ptr(vec4(1.0, 0.0, 0.0, 1.0)));
    glUniform4fv(COLOR2_LOC, 1, value_ptr(vec4(0.0, 1.0, 0.0, 1.0)));

    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, matricesubo);
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glUseProgram(0);
  }

  void draw(SDLGLWindow& w) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(vao1);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere_mesh.ibo);
    glUseProgram(phongProgram);
    glDrawElements(GL_TRIANGLES, sphere_mesh.num_indices, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

//    glBindVertexArray(vao2);
//    glUseProgram(drawNormalsProgram);
//    glDrawArrays(GL_LINES, 0, sphere_mesh.num_vertices * 2);
//    glBindVertexArray(0);
//    glUseProgram(0);
  }
};

int main(int argc, char** argv) {
  SimpleMirrorGLWindow w(1024, 768);
  w.mainLoop();
}
