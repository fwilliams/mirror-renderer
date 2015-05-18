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
      std::array<glm::ivec2, vertsPerTile<TYPE>()>adjacentVerticesForTile(const glm::ivec2& tile) const;
      std::array<glm::ivec2, vertsPerTile<TYPE>()> adjacentTilesForTile(const glm::ivec2& tile) const;
      std::array<glm::ivec2, tilesPerVertex<TYPE>()> adjacentTilesForVertex(const glm::ivec2& vertex) const;
      std::array<glm::ivec2, tilesPerVertex<TYPE>()> adjacentVerticesForVertex(const glm::ivec2& vertex) const;

      static size_t const NUM_ADJ_VERTS_PER_TILE = vertsPerTile<TYPE>();
      static size_t const NUM_ADJ_TILES_PER_TILE = vertsPerTile<TYPE>();
      static size_t const NUM_ADJ_TILES_PER_VERT = tilesPerVertex<TYPE>();
      static size_t const NUM_ADJ_VERTS_PER_VERT = tilesPerVertex<TYPE>();
    };

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::adjacentVerticesForTile(const glm::ivec2 &tile) const {
      return {tile, right(tile), upright(tile), up(tile)};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::adjacentTilesForTile(const glm::ivec2 &tile) const {
      return {right(tile), up(tile), left(tile), down(tile)};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::adjacentTilesForVertex(const glm::ivec2 &v) const {
      return {v, left(v), downleft(v), down(v) };
    }

    template<>
    std::array<glm::ivec2, tilesPerVertex<PlanarTileType::QUAD>()>
    TileTopologyPolicy<PlanarTileType::QUAD>::adjacentVerticesForVertex(const glm::ivec2& v) const {
      return {up(v), right(v), down(v), left(v)};
    }



    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::adjacentVerticesForTile(const glm::ivec2 &tile) const {
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
    TileTopologyPolicy<PlanarTileType::TRI>::adjacentTilesForTile(const glm::ivec2 &tile) const {
      if (tile.y % 2 == 0) {
        return {down(tile), upright(tile), up(tile)};
      } else {
        return {down(tile), up(tile), downleft(tile)};
      }
    }

    template<>
    std::array<glm::ivec2, tilesPerVertex<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::adjacentTilesForVertex(const glm::ivec2& v) const {
      return { v, up(v), left(v), downleft(v), down(downleft(v)), down(v) };
    }

    template<>
    std::array<glm::ivec2, tilesPerVertex<PlanarTileType::TRI>()>
    TileTopologyPolicy<PlanarTileType::TRI>::adjacentVerticesForVertex(const glm::ivec2& v) const {
      return {up(v), upright(v), right(v), down(v), downleft(v), left(v)};
    }



    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy<PlanarTileType::HEX>::adjacentVerticesForTile(const glm::ivec2 &tile) const {
      glm::ivec2 t(2*tile.x+tile.y, tile.x + 2*tile.y);
      return { t, right(t), glm::ivec2(t.x+2, t.y+1),
               glm::ivec2(t.x+2, t.y+2), glm::ivec2(t.x+1, t.y+2), up(t) };
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy<PlanarTileType::HEX>::adjacentTilesForTile(const glm::ivec2 &v) const {
      return {};
    }

    template<>
    std::array<glm::ivec2, tilesPerVertex<PlanarTileType::HEX>()>
    TileTopologyPolicy<PlanarTileType::HEX>::adjacentVerticesForVertex(const glm::ivec2& v) const {
      return {};
    }



    template<class TYPE, class TOPOLOGY>
    class PlanarTileMap : private TOPOLOGY {
    public:
      struct Vertex;
      struct Tile;

      struct Tile {
        glm::ivec2 id;
        size_t adjacentTileSize = 0;

        size_t numAdjacentTiles() const {
          return adjacentTileSize;
        }

        glm::ivec2 tileId() const {
          return id;
        }

        std::array<Tile*, TOPOLOGY::NUM_ADJ_TILES_PER_TILE> adjacentTiles;
        std::array<Vertex*, TOPOLOGY::NUM_ADJ_VERTS_PER_TILE> adjacentVertices;

        typedef typename decltype(adjacentVertices)::iterator vertexiterator;
        typedef typename decltype(adjacentVertices)::iterator tileiterator;

        tileiterator tiles_begin() const { return adjacentTiles.begin(); }
        tileiterator tiles_end() const { return adjacentTiles.begin() + adjacentTileSize; }

        vertexiterator vertices_begin() const { return adjacentVertices.begin(); }
        vertexiterator vertices_end() const { return adjacentVertices.end(); }

        TYPE data;
      };

      struct Vertex {
        glm::ivec2 id;
        size_t adjacentTileSize = 0;
        size_t adjacentVertsSize = 0;

        size_t numAdjacentTiles() const {
          return adjacentTileSize;
        }

        size_t numAdjacentVertices() const {
          return adjacentVertsSize;
        }

        glm::ivec2 vertexId() const {
          return id;
        }

        std::array<Tile*, TOPOLOGY::NUM_ADJ_TILES_PER_VERT> adjacentTiles;
        std::array<Vertex*, TOPOLOGY::NUM_ADJ_VERTS_PER_VERT> adjacentVertices;

        typedef typename decltype(adjacentVertices)::iterator vertexiterator;
        typedef typename decltype(adjacentVertices)::iterator tileiterator;

        vertexiterator vertices_begin() const { return adjacentVertices.begin(); }
        vertexiterator vertices_end() const { return adjacentVertices.begin() + adjacentVertsSize; }

        tileiterator tiles_begin() const { return adjacentTiles.begin(); }
        tileiterator tiles_end() const { return adjacentTiles.begin() + adjacentTileSize; }
      };

    private:
      std::unordered_map<glm::ivec2, Tile> tiles;
      std::unordered_map<glm::ivec2, Vertex> verts;

      Tile* findTilesDFS(const glm::ivec2 &tile, const std::function<bool(const glm::ivec2 &)> &pred) {
        // Insert the new tile into the tile map
        auto tileData = &tiles[tile];
        tileData->id = tile;

        auto adjacentVertIds = this->adjacentVerticesForTile(tile);
        auto adjacentTileIds = this->adjacentTilesForTile(tile);

        // Insert adjacent tiles into tile map and update Tile data structure
        for(auto i = adjacentTileIds.begin(); i != adjacentTileIds.end(); i++) {
          auto tile = tiles.find(*i);
          if(tile == tiles.end()) {
            if(pred(*i)) {
              Tile* t =  findTilesDFS(*i, pred);
              tileData->adjacentTiles[tileData->adjacentTileSize++] = t;
            }
          } else {
            tileData->adjacentTiles[tileData->adjacentTileSize++] = &tile->second;
          }
        }

        // Insert adjacent vertices for tile into vertex map
        {
          size_t numVerticesInserted = 0;
          for(auto i = adjacentVertIds.begin(); i != adjacentVertIds.end(); i++) {
            auto vAdjacentVertices = this->adjacentVerticesForVertex(*i);

            bool v1New = verts.find(*i) == verts.end();
            Vertex* v1 = &verts[*i];
            v1->id = *i;

            for(auto j = vAdjacentVertices.begin(); j != vAdjacentVertices.end(); j++) {
              bool v2New = verts.find(*j) == verts.end();

              if((v1New && !v2New) || (!v1New && v2New)) {
                Vertex* v2 = &verts[*j];
                v2->id = *j;

                v1->adjacentVertices[v1->adjacentVertsSize++] = v2;
                v2->adjacentVertices[v2->adjacentVertsSize++] = v1;
              }
            }

            tileData->adjacentVertices[numVerticesInserted++] = v1;
            v1->adjacentTiles[v1->adjacentTileSize++] = tileData;
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
