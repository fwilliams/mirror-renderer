#include <GL/glew.h>
#include <stdio.h>
#include <vector>
#include <type_traits>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#ifndef GEOMETRY_H_
#define GEOMETRY_H_

struct vertex {
  glm::vec4 position;
  glm::vec3 normal;
  glm::vec2 texcoord;
};

struct Geometry {
  Geometry() {}
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
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*) sizeof(vao));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
        (void*) (sizeof(glm::vec4) + sizeof(glm::vec3)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
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

  static Geometry make_triangle() {
    Geometry ret(3, 3);

    // Allocate and map a vertex and index buffer object for the sphere
    ret.num_vertices = 3;
    ret.num_indices = 3;

    glGenBuffers(1, &ret.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glBufferData(GL_ARRAY_BUFFER, ret.num_vertices * sizeof(vertex), nullptr, GL_STATIC_DRAW);

    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glGenBuffers(1, &ret.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ret.num_indices * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

    GLuint* indices = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    vertices[0].position = {-0.5, -0.5, 0.0, 1.0 };
    vertices[1].position = { 0.5, -0.5, 0.0, 1.0 };
    vertices[2].position = { 0.0,  0.5, 0.0, 1.0 };
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;

    compute_normals(vertices, indices, ret.num_indices);

    glGenBuffers(1, &ret.normal_view_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret.normal_view_vbo);
    glBufferData(GL_ARRAY_BUFFER, ret.num_vertices * 2 * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);
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

    // Allocate and map a vertex and index buffer object for the sphere
    ret.num_vertices = theta_samples * phi_samples;
    ret.num_indices = phi_samples * 6 + (theta_samples - 2) * phi_samples * 6;

    glGenBuffers(1, &ret.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    glBufferData(GL_ARRAY_BUFFER, ret.num_vertices * sizeof(vertex), nullptr, GL_STATIC_DRAW);

    vertex* vertices = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glGenBuffers(1, &ret.ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ret.num_indices * sizeof(GLuint), nullptr, GL_STATIC_DRAW);

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

  //  double cos_sin_phi[2][phi_samples];
    // Cache of cosine and sine of each phi sample for reuse in inner loop
    double* cos_sin_phi[2];
    {
      double* arr = new double[2*phi_samples];
      cos_sin_phi[0] = arr;
      cos_sin_phi[1] = &arr[phi_samples];
    }


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

    glGenBuffers(1, &ret.normal_view_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ret.normal_view_vbo);
    glBufferData(GL_ARRAY_BUFFER, ret.num_vertices * 2 * sizeof(glm::vec4), nullptr, GL_STATIC_DRAW);
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

    return ret;
  }
};


#endif /* GEOMETRY_H_ */
