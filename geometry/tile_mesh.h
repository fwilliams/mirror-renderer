#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include <string>
#include <array>
#include <unordered_map>

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "geometry/planar_tiling.h"
#include "geometry/3d_primitives.h"
#include "geometry/vertex.h"

#ifndef TILEMESH_H_
#define TILEMESH_H_


namespace geometry {

struct Quad4 {
  glm::vec4 p1, p2, p3, p4;

  Quad4() = default;
  Quad4(const glm::vec4& p1, const glm::vec4& p2, const glm::vec4& p3, const glm::vec4& p4) : p1(p1), p2(p2), p3(p3), p4(p4) {}

  glm::vec3 normal() {
    return glm::cross(glm::vec3(p2-p1), glm::vec3(p4-p1));
  }

  // TODO: Implement this
  glm::vec2 centerUV() {
    return glm::vec2(0.0);
  }

  glm::vec4 center() {
    return (p1 + p2 + p3 + p4)/ 4.0f;
  }

  bool isDegenerate() {
    glm::vec4 e1 = p2 - p1;
    glm::vec4 e2 = p4 - p1;
    glm::vec4 e3 = p2 - p3;
    glm::vec4 e4 = p4 - p3;

    return dot(e1, e2) == 0.0 || dot(e3, e4) == 0.0;
  }

  Quad4& operator+=(const Quad4& quad) {
    p1 += quad.p1;
    p2 += quad.p2;
    p3 += quad.p3;
    p4 += quad.p4;
    return *this;
  }

  Quad4& operator-=(const Quad4& quad) {
    p1 -= quad.p1;
    p2 -= quad.p2;
    p3 -= quad.p3;
    p4 -= quad.p4;
    return *this;
  }
};

Quad4 operator*(const Quad4& quad, const glm::mat4& tx) {
  return Quad4{quad.p1*tx, quad.p2*tx, quad.p3*tx, quad.p4*tx};
}

Quad4 operator*(const glm::mat4& tx, const Quad4& quad) {
  return Quad4{tx*quad.p1, tx*quad.p2, tx*quad.p3, tx*quad.p4};
}

Quad4 operator+(const Quad4& quad1, const Quad4& quad2) {
  return Quad4{quad1.p1+quad2.p1, quad1.p2+quad2.p2, quad1.p2+quad2.p3, quad1.p2+quad2.p4};
}

Quad4 operator-(const Quad4& quad1, const Quad4& quad2) {
  return Quad4{quad1.p1-quad2.p1, quad1.p2-quad2.p2, quad1.p2-quad2.p3, quad1.p2-quad2.p4};
}


template <class Tiling>
class TileMesh {
public:
  typedef Tuple<glm::vec4, glm::vec3, glm::vec3> Vertex;
private:
  enum {POS = 0, TEX = 1, ID = 2};
  Tiling mTiling;

  bool mRebuildGeometry = true;

  Geometry mGeometry;

  GLuint mTileTextureArray = 0;
  GLuint mTileDepthTextureArray = 0;
  GLuint mNumTextures = 0;

  void depthsort(Vertex* verts, GLuint* inds, size_t numIndices);

  /*
   * Create an OpenGL texture2D array of n textures of size w by h pixels each
   */
  GLuint makeTextureArray(size_t w, size_t h, size_t n);

  /*
   * Load the image in the file whose name is key to arrayIndex in the Texture2DArray
   * specified by tex
   */
  void loadImgToTexArray(const std::string& key, GLuint tex, size_t arrayIndex);

  std::pair<std::string, std::string> getTexKey(const glm::ivec2& tileIndex, const glm::ivec2& adjacentTileIndex);

  Geometry generateTileGeometry();

public:
  void printTextureNames();

  static std::array<Quad4, Tiling::numEdgesPerTile()> edgeFaces() {
    std::array<Quad4, Tiling::numEdgesPerTile()> ret;

    auto adjVerts = Tiling::adjacentVertices(glm::ivec2(0));
    const glm::vec2 TILE_CENTER_OFFSET = Tiling::tileCenterCoords2d(glm::ivec2(0));

    for(size_t i = 0; i != Tiling::numEdgesPerTile(); i++) {
      glm::vec2 v1 = Tiling::coords2d(adjVerts[i]) - TILE_CENTER_OFFSET;
      glm::vec2 v2 = Tiling::coords2d(adjVerts[(i+1) % Tiling::numEdgesPerTile()]) - TILE_CENTER_OFFSET;
      ret[i] = Quad4(
          glm::vec4(v1.x, -0.5, v1.y, 1.0),
          glm::vec4(v2.x, -0.5, v2.y, 1.0),
          glm::vec4(v2.x, 0.5, v2.y, 1.0),
          glm::vec4(v1.x, 0.5, v1.y, 1.0));
    }

    return ret;
  }

  GLuint numTextures() const {
    return mNumTextures;
  }

  GLuint tileTextureArray() const {
    return mTileTextureArray;
  }

  GLuint tileDepthTextureArray() const {
    return mTileDepthTextureArray;
  }

  const Geometry& geometry() {
    if(mRebuildGeometry) {
      mGeometry = generateTileGeometry();
      mRebuildGeometry = false;
    }
    return mGeometry;
  }

  void rebuildMesh(size_t radius, const glm::ivec2& ctr = glm::ivec2(0));

  TileMesh(size_t radius);

  virtual ~TileMesh();
};

#ifndef IS_IDE
#include "tile_mesh.inl.cpp"
#endif

}
#endif /* TILEMESH_H_ */
