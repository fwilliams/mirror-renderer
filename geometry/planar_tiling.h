#include <unordered_set>
#include <unordered_map>
#include <array>
#include <functional>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include "utils/glm_hash.hpp"

#ifndef GEOMETRY_PLANAR_TILING_H_
#define GEOMETRY_PLANAR_TILING_H_

namespace geometry {
  inline glm::ivec2 right(const glm::ivec2 &v) {
    return glm::ivec2(v.x + 1, v.y);
  }

  inline glm::ivec2 left(const glm::ivec2 &v) {
    return glm::ivec2(v.x - 1, v.y);
  }

  inline glm::ivec2 up(const glm::ivec2 &v) {
    return glm::ivec2(v.x, v.y + 1);
  }

  inline glm::ivec2 down(const glm::ivec2 &v) {
    return glm::ivec2(v.x, v.y - 1);
  }

  inline glm::ivec2 downleft(const glm::ivec2 &v) {
    return glm::ivec2(v.x - 1, v.y - 1);
  }

  inline glm::ivec2 downright(const glm::ivec2 &v) {
    return glm::ivec2(v.x + 1, v.y - 1);
  }

  inline glm::ivec2 upleft(const glm::ivec2 &v) {
    return glm::ivec2(v.x - 1, v.y + 1);
  }

  inline glm::ivec2 upright(const glm::ivec2 &v) {
    return glm::ivec2(v.x + 1, v.y + 1);
  }

  namespace detail {

    enum class PlanarTileType {
      QUAD, TRI, HEX
    };

    template<PlanarTileType T>
    constexpr unsigned vertsPerTile();

    template<>
    constexpr unsigned vertsPerTile<PlanarTileType::TRI>() { return 3; }

    template<>
    constexpr unsigned vertsPerTile<PlanarTileType::QUAD>() { return 4; }

    template<>
    constexpr unsigned vertsPerTile<PlanarTileType::HEX>() { return 6; }


    template<PlanarTileType T>
    constexpr unsigned tilesPerVertex();

    template<>
    constexpr unsigned tilesPerVertex<PlanarTileType::TRI>() { return 6; }

    template<>
    constexpr unsigned tilesPerVertex<PlanarTileType::QUAD>() { return 4; }

    template<>
    constexpr unsigned tilesPerVertex<PlanarTileType::HEX>() { return 3; }

    template <PlanarTileType TYPE>
    struct TileTopologyPolicy {
      std::array<glm::ivec2, vertsPerTile<TYPE>()>verticesForTile(const glm::ivec2& tile);
      std::array<glm::ivec2, vertsPerTile<TYPE>()> tAdjacentTiles(const glm::ivec2& tile);
      std::array<glm::ivec2, tilesPerVertex<TYPE>()> vAdjacentTiles(const glm::ivec2& vertex);

      static size_t const VERTEX_COUNT = vertsPerTile<TYPE>();
      static size_t const ADJ_FACE_COUNT = vertsPerTile<TYPE>();
    };

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::verticesForTile(const glm::ivec2 &tile) {
      return {tile, right(tile), upright(tile), up(tile)};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::tAdjacentTiles(const glm::ivec2 &tile) {
      return {right(tile), up(tile), left(tile), down(tile)};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::vAdjacentTiles(const glm::ivec2 &v) {
      return {v, left(v), downleft(v), down(v) };
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::verticesForTile(const glm::ivec2 &tile) {
      if (tile.y % 2 == 0) {
        glm::ivec2 t(tile.x, tile.y / 2);
        return {t, right(t), upright(t)};
      } else {
        glm::ivec2 t(tile.x, tile.y / 2 + 1);
        return {t, down(t), right(t)};
      }
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::tAdjacentTiles(const glm::ivec2 &tile) {
      if (tile.y % 2 == 0) {
        return {down(tile), upright(tile), up(tile)};
      } else {
        return {down(tile), up(tile), downleft(tile)};
      }
    }

    template<>
    std::array<glm::ivec2, tilesPerVertex<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::vAdjacentTiles(const glm::ivec2& v) {
      return { v, up(v), left(v), downleft(v), down(downleft(v)), down(v) };
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy<PlanarTileType::HEX>::verticesForTile(const glm::ivec2 &tile) {
      glm::ivec2 t(2*tile.x+tile.y, tile.x + 2*tile.y);
      return { t, right(t), glm::ivec2(t.x+2, t.y+1),
               glm::ivec2(t.x+2, t.y+2), glm::ivec2(t.x+1, t.y+2), up(t) };
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy<PlanarTileType::HEX>::tAdjacentTiles(const glm::ivec2 &v) {
      return {};
    }

    template<class TYPE, class TOPOLOGY>
    class PlanarTileMap : private TOPOLOGY {
    public:
      struct Tile {
        glm::ivec2 key;
        size_t adjTileSize = 0;

        size_t numAdjacentTiles() const {
          return adjTileSize;
        }

        glm::ivec2 tileId() const {
          return key;
        }

        std::array<glm::ivec2, TOPOLOGY::VERTEX_COUNT> vertices;
        std::array<const Tile*, TOPOLOGY::ADJ_FACE_COUNT> adjacentTiles;
        typedef typename decltype(vertices)::iterator vertexiterator;
        typedef typename decltype(vertices)::iterator tileiterator;

        vertexiterator vertices_begin() const { return vertices.begin(); }
        tileiterator tiles_begin() const { return tiles.begin(); }
        vertexiterator vertices_end() const { return vertices.end(); }
        tileiterator tiles_end() const { return tiles.begin() + adjTileSize; }

        TYPE data;
      };

    private:
      std::unordered_map<glm::ivec2, Tile> tiles;
      std::unordered_set<glm::ivec2> verts;

      Tile* findTilesDFS(const glm::ivec2 &tile, const std::function<bool(const glm::ivec2 &)> &pred) {
        auto tileData = &tiles[tile];
        tileData->vertices = this->verticesForTile(tile);

        auto adjTileInds = this->tAdjacentTiles(tile);

        verts.insert(tileData->vertices.begin(), tileData->vertices.end());

        for(auto i = adjTileInds.begin(); i != adjTileInds.end(); i++) {
          if(tiles.find(*i) == tiles.end() && pred(*i)) {
            tileData->adjacentTiles[tileData->adjTileSize++] = findTilesDFS(*i, pred);
          }
        }

        return tileData;
      }

    public:

      Tile* addTilesInNeighborhood(glm::ivec2 point, const std::function<bool(const glm::ivec2 &)> &pred) {
        if (tiles.find(point) == tiles.end() && pred(point)) {
          return findTilesDFS(point, pred);
        }
        return nullptr;
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
    };

    struct EmptyStruct {};

    template <PlanarTileType T>
    using PlanarTileSet = PlanarTileMap<EmptyStruct, TileTopologyPolicy<T>>;
  }

  using QuadPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::QUAD>;
  using TriPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::TRI>;
  using HexPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::HEX>;

  template <typename T>
  using QuadPlanarTileMap = detail::PlanarTileMap<T, detail::TileTopologyPolicy<detail::PlanarTileType::QUAD>>;

  template <typename T>
  using TriPlanarTileMap = detail::PlanarTileMap<T, detail::TileTopologyPolicy<detail::PlanarTileType::TRI>>;

  template <typename T>
  using HexPlanarTileMap = detail::PlanarTileMap<T, detail::TileTopologyPolicy<detail::PlanarTileType::HEX>>;
}

#endif /* GEOMETRY_PLANAR_TILING_H_ */
