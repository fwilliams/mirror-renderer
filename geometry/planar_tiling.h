#include <unordered_set>
#include <unordered_map>
#include <array>
#include <functional>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "../utils/glm_hash.hpp"

#ifndef GEOMETRY_PLANAR_TILING2_H_
#define GEOMETRY_PLANAR_TILING2_H_

enum class PlanarTileType {
  QUAD, TRI, HEX
};

template <PlanarTileType T>
constexpr unsigned vertsPerTile();

template <>
constexpr unsigned vertsPerTile<PlanarTileType::TRI>() { return 3; }

template <>
constexpr unsigned vertsPerTile<PlanarTileType::QUAD>() { return 4; }

template <>
constexpr unsigned vertsPerTile<PlanarTileType::HEX>() { return 6; }

template <PlanarTileType T>
class PlanarTiling {
  std::unordered_map<glm::ivec2, std::array<glm::ivec2, vertsPerTile<T>()>> tiles;
  std::unordered_set<glm::ivec2> verts;

  std::array<glm::ivec2, vertsPerTile<T>()> verticesForTile(const glm::ivec2& tile);

  void findTilesDFS(const glm::ivec2& tile, const std::function<bool(const glm::ivec2&)>& pred) {
    auto vertices = tiles[tile];
    vertices = verticesForTile(tile);
    verts.insert(vertices.begin(), vertices.end());

    if(tiles.find(right(tile)) == tiles.end() && pred(right(tile))) {
      findTilesDFS(right(tile), pred);
    }
    if(tiles.find(up(tile)) == tiles.end() && pred(up(tile))) {
      findTilesDFS(up(tile), pred);
    }
    if(tiles.find(left(tile)) == tiles.end() && pred(left(tile))) {
      findTilesDFS(left(tile), pred);
    }
    if(tiles.find(down(tile)) == tiles.end() && pred(down(tile))) {
      findTilesDFS(down(tile), pred);
    }
  }

public:
  void addTilesInNeighborhood(glm::ivec2 point, const std::function<bool(const glm::ivec2&)>& pred) {
    if(tiles.find(point) == tiles.end() && pred(point)) {
      findTilesDFS(point, pred);
    }
  }

  size_t edgeCount() const {
    return vertexCount() + tileCount() - 1;
  }

  size_t vertexCount() const {
    return verts.size();
  }

  size_t tileCount() const {
    return tiles.size();
  }

  inline static glm::ivec2 right(const glm::ivec2& v) {
    return glm::ivec2(v.x+1, v.y);
  }

  inline static glm::ivec2 left(const glm::ivec2& v) {
    return glm::ivec2(v.x-1, v.y);
  }

  inline static glm::ivec2 up(const glm::ivec2& v) {
    return glm::ivec2(v.x, v.y+1);
  }

  inline static glm::ivec2 down(const glm::ivec2& v) {
    return glm::ivec2(v.x, v.y-1);
  }

  inline static glm::ivec2 downleft(const glm::ivec2& v) {
    return glm::ivec2(v.x-1, v.y-1);
  }

  inline static glm::ivec2 downright(const glm::ivec2& v) {
    return glm::ivec2(v.x+1, v.y-1);
  }

  inline static glm::ivec2 upleft(const glm::ivec2& v) {
    return glm::ivec2(v.x-1, v.y+1);
  }

  inline static glm::ivec2 upright(const glm::ivec2& v) {
    return glm::ivec2(v.x+1, v.y+1);
  }
};


template <>
std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
PlanarTiling<PlanarTileType::QUAD>::verticesForTile(const glm::ivec2& tile) {
  return { tile, right(tile), upright(tile), up(tile) };
}

template <>
std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
PlanarTiling<PlanarTileType::TRI>::verticesForTile(const glm::ivec2& tile) {
  if(tile.y % 2 == 0) {
    glm::ivec2 t(tile.x, tile.y/2);
    return { t, right(t), upright(t) };
  } else {
    glm::ivec2 t(tile.x, tile.y/2 + 1);
    return { t, down(t), right(t) };
  }
}

template <>
std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
PlanarTiling<PlanarTileType::HEX>::verticesForTile(const glm::ivec2& tile) {
  glm::ivec2 t = tile * glm::ivec2(2, 1);
  return { t, right(t), upright(t),
           glm::ivec2(t.x, t.y+2), glm::ivec2(t.x-1, t.y+2), up(t) };
}

#endif /* GEOMETRY_PLANAR_TILING2_H_ */
