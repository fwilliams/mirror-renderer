#include <GL/glew.h>
#include <stdio.h>

#include <vector>
#include <array>
#include <algorithm>
#include <type_traits>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#include "gl_utils.h"

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

struct vertex {
  glm::vec4 position;
  glm::vec3 normal;
  glm::vec2 texcoord;
};

class Geometry {
  Geometry(GLuint num_vertices, GLuint num_indices) : num_vertices(num_vertices), num_indices(num_indices) {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(vertex), nullptr, GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vertex), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) sizeof(glm::vec4));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
        (void*) (sizeof(glm::vec4) + sizeof(glm::vec3)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &normal_view_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, normal_view_vbo);
    glBufferData(GL_ARRAY_BUFFER, num_vertices * 2 * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);

    glGenVertexArrays(1, &normal_view_vao);
    glBindVertexArray(normal_view_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

public:
  Geometry() {}
  ~Geometry() {
//    glDeleteBuffers(1, &vbo);
//    glDeleteBuffers(1, &ibo);
//    glDeleteBuffers(1, &normal_view_vbo);
//    glDeleteVertexArrays(1, &vao);
//    glDeleteVertexArrays(1, &normal_view_vao);
  }

  GLuint vbo = 0;
  GLuint ibo = 0;
  GLuint vao = 0;
  GLuint normal_view_vbo = 0;
  GLuint normal_view_vao = 0;
  size_t num_vertices = 0;
  size_t num_indices = 0;

  static void compute_normals(vertex* vertices, GLuint* indices, size_t num_indices) {
    // Compute the normals of each triangle
    for(unsigned i = 0; i < num_indices; i += 3) {
      vertex* verts [] {
        &vertices[indices[i]], &vertices[indices[i+1]], &vertices[indices[i+2]]
      };

      glm::vec3 v1 = glm::vec3(verts[0]->position - verts[1]->position);
      glm::vec3 v2 = glm::vec3(verts[2]->position - verts[1]->position);
      glm::vec3 normal = glm::normalize(glm::cross(v1, v2));

      for(unsigned j = 0; j < 3; j++) {
        verts[j]->normal = glm::normalize((verts[j]->normal + normal) / 2.0f);
      }
    }
  }

  static Geometry make_cube(const glm::vec3& scale, bool invertNormals = false) {
    Geometry ret(8, 36);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ARRAY_BUFFER, ret.ibo);
    GLuint* indices = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));


    const float normalScale = invertNormals ? -1.0 : 1.0;
    const glm::vec4 v4scale(scale, 1.0);

    const std::array<glm::vec4, 8> verts {
      glm::vec4(-0.5, -0.5,  0.5, 1.0),
      glm::vec4( 0.5, -0.5,  0.5, 1.0),
      glm::vec4( 0.5,  0.5,  0.5, 1.0),
      glm::vec4(-0.5,  0.5,  0.5, 1.0),
      glm::vec4(-0.5, -0.5, -0.5, 1.0),
      glm::vec4( 0.5, -0.5, -0.5, 1.0),
      glm::vec4( 0.5,  0.5, -0.5, 1.0),
      glm::vec4(-0.5,  0.5, -0.5, 1.0)
    };

    const std::array<glm::vec3, 8> norms {
      glm::vec3(-0.577350, -0.577350,  0.577350),
      glm::vec3( 0.577350, -0.577350,  0.577350),
      glm::vec3( 0.577350,  0.577350,  0.577350),
      glm::vec3(-0.577350,  0.577350,  0.577350),
      glm::vec3(-0.577350, -0.577350, -0.577350),
      glm::vec3( 0.577350, -0.577350, -0.577350),
      glm::vec3( 0.577350,  0.577350, -0.577350),
      glm::vec3(-0.577350,  0.577350, -0.577350),
    };

    const std::array<GLuint, 36> inds {
      0, 1, 2, 2, 3, 0,
      3, 2, 6, 6, 7, 3,
      7, 6, 5, 5, 4, 7,
      4, 0, 3, 3, 7, 4,
      1, 0, 5, 4, 5, 0,
      1, 5, 6, 6, 2, 1
    };

    for(size_t i = 0; i < verts.size(); i++) {
      vertices[i].position = v4scale * verts[i];
      vertices[i].normal = normalScale * norms[i];
    }
    std::copy(inds.begin(), inds.end(), indices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.normal_view_vbo);
    glm::vec4* normals = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    for(unsigned j = 0, i = 0; j < ret.num_vertices; i+=2) {
      normals[i] = vertices[j].position;
      normals[i+1] = normals[i] + glm::vec4(vertices[j].normal, 1.0f);
      normals[i].w = 0.0;
      normals[i+1].w = 1.0;
      j+=1;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  static Geometry make_plane(unsigned uSamples, unsigned vSamples) {
    unsigned numVertices = (uSamples+1) * (vSamples+1);
    unsigned numIndices = uSamples * vSamples * 6;

    Geometry ret(numVertices, numIndices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* indices = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    for(unsigned i = 0; i <= uSamples; i++) {
      for(unsigned j = 0; j <= vSamples; j++) {
        const glm::vec2 vPos = (glm::vec2(i, j) -
                                glm::vec2(static_cast<float>(uSamples)/2.0,
                                         static_cast<float>(vSamples)/2.0)) *
                               glm::vec2(1.0/(uSamples+1), 1.0/(vSamples+1));
        std::cout << glm::to_string(vPos) << std::endl;
        const size_t vOffset = i*(uSamples + 1) + j;

        vertices[vOffset].position = glm::vec4(vPos, 0.0, 1.0);
        vertices[vOffset].texcoord = vPos + glm::vec2(0.5);
        vertices[vOffset].normal = glm::vec3(0.0, 0.0, -1.0);
      }
    }

    for(unsigned i = 0; i < uSamples; i++) {
      for(unsigned j = 0; j < vSamples; j++) {
        const size_t iOffset = (i*uSamples + j) * 6;
        const size_t iBase = i*(uSamples+1) + j;

        indices[iOffset + 0] = iBase;
        indices[iOffset + 1] = iBase + 1;
        indices[iOffset + 2] = iBase + uSamples + 1;

        indices[iOffset + 3] = iBase + 1;
        indices[iOffset + 4] = iBase + uSamples + 2;
        indices[iOffset + 5] = iBase + uSamples + 1;
      }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  static Geometry make_triangle() {
    Geometry ret(3, 3);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* indices = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    vertices[0].position = {-0.5, -0.5, 0.0, 1.0 };
    vertices[1].position = { 0.5, -0.5, 0.0, 1.0 };
    vertices[2].position = { 0.0,  0.5, 0.0, 1.0 };
    vertices[0].normal = { 0.0, 0.0, 1.0 };
    vertices[1].normal = { 0.0, 0.0, 1.0 };
    vertices[2].normal = { 0.0, 0.0, 1.0 };
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;


    compute_normals(vertices, indices, ret.num_indices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.normal_view_vbo);
    glm::vec4* normals = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    for(unsigned j = 0, i = 0; j < ret.num_vertices; i+=2) {
      normals[i] = vertices[j].position;
      normals[i+1] = normals[i] - (0.2f * glm::vec4(vertices[j].normal, 1.0f));
      normals[i+1].w = 0.0;
      normals[i].w = 1.0;
      j+=1;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  static Geometry make_sphere(double radius, unsigned theta_samples, unsigned phi_samples) {
    Geometry ret(theta_samples * phi_samples, phi_samples * 6 + (theta_samples - 2) * phi_samples * 6);

    // Map vertex and index buffer objects for the sphere
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* indices = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    // Variables hold the current vertex to write to
    unsigned vert_i = 0, ind_i = 0;

    // Angular distances between samples
    const double d_theta = glm::pi<double>() / theta_samples;
    const double d_phi = glm::two_pi<double>() / phi_samples;

    // Angular representation of current theta sample
    double theta = -glm::half_pi<double>() + d_theta;

    // Radius of the circle for the current theta sample
    double xz_rad = radius * glm::cos(theta);

    // Cartesian y coordinate representation of current theta sample
    double y = radius * glm::sin(theta);

    // Offset of the first index for this theta sample
    unsigned index_base = 1;

    // Cache of cosine and sine of each phi sample for reuse in inner loop
    double* cos_sin_phi[2];
    cos_sin_phi[0] = new double[2*phi_samples];
    cos_sin_phi[1] = &(cos_sin_phi[0][phi_samples]);


    // Set the position of the top vertex
    vertices[vert_i].position = {0.0, -radius, 0.0, 1.0};
    vertices[vert_i].normal = glm::normalize(glm::vec3(vertices[vert_i].position));
    vert_i += 1;

    // Construct the top triangle of vertices connected to the top vertex
    for (unsigned j = 0; j < phi_samples; j++) {
      double phi = j * d_phi;
      cos_sin_phi[0][j] = glm::cos(phi);
      cos_sin_phi[1][j] = glm::sin(phi);
      vertices[vert_i].position = {xz_rad * cos_sin_phi[0][j], y, xz_rad * cos_sin_phi[1][j], 1.0};
      vertices[vert_i].normal = glm::normalize(glm::vec3(vertices[vert_i].position));
      vert_i += 1;

      indices[ind_i++] = 0;
      indices[ind_i++] = index_base + j;
      indices[ind_i++] = index_base + ((j + 1) % phi_samples);
    }

    // Construct all inner triangles not including the top and bottom vertex
    for (unsigned i = 0; i < (theta_samples - 2); i++) {
      theta += d_theta;
      xz_rad = radius * glm::cos(theta);
      y = radius * glm::sin(theta);

      for (unsigned j = 0; j < phi_samples; j++) {
        vertices[vert_i].position = {xz_rad * cos_sin_phi[0][j], y, xz_rad * cos_sin_phi[1][j], 1.0};
        vertices[vert_i].normal = glm::normalize(glm::vec3(vertices[vert_i].position));
        vert_i += 1;

        indices[ind_i++] = index_base + j;
        indices[ind_i++] = index_base + j + phi_samples;
        indices[ind_i++] = index_base + ((j + 1) % phi_samples) + phi_samples;

        indices[ind_i++] = index_base + j;
        indices[ind_i++] = index_base + ((j + 1) % phi_samples) + phi_samples;
        indices[ind_i++] = index_base + ((j + 1) % phi_samples);
      }
      index_base += phi_samples;
    }

    // Construct the bottom triangles connecting to the last vertex
    const unsigned LAST_INDEX = index_base + phi_samples;
    for (unsigned j = 0; j < phi_samples; j++) {
      indices[ind_i++] = index_base + j;
      indices[ind_i++] = LAST_INDEX;
      indices[ind_i++] = index_base + ((j + 1) % phi_samples);
    }

    // Set the position of the bottom vertex
    vertices[vert_i].position = {0.0, radius, 0.0, 1.0};
    vertices[vert_i].normal = glm::normalize(glm::vec3(vertices[vert_i].position));
    vert_i += 1;

    glBindBuffer(GL_ARRAY_BUFFER, ret.normal_view_vbo);
    glm::vec4* normals = reinterpret_cast<glm::vec4*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    for(unsigned j = 0, i = 0; j < ret.num_vertices; i+=2) {
      normals[i] = vertices[j].position;
      normals[i+1] = normals[i] + ((static_cast<float>(radius)/3.0f) * glm::vec4(vertices[j].normal, 1.0f));
      normals[i].w = 0.0;
      normals[i+1].w = 1.0;
      j+=1;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    delete cos_sin_phi[0];
    return ret;
  }
};


#endif /* GEOMETRY_H_ */
