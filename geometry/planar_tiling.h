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
    struct TileTopologyPolicy2 {
      std::array<glm::ivec2, vertsPerTile<TYPE>()> adjacentTiles(const glm::ivec2& tile) const;
      std::array<glm::ivec2, vertsPerTile<TYPE>()> adjacentVertices(const glm::ivec2& tile) const;
      std::array<glm::ivec2, vertsPerTile<TYPE>()> edges(const glm::ivec2& tile) const;

      static size_t const NUM_ADJ_VERTS_PER_TILE = vertsPerTile<TYPE>();
      static size_t const NUM_ADJ_TILES_PER_TILE = vertsPerTile<TYPE>();
      static size_t const NUM_ADJ_TILES_PER_VERT = tilesPerVertex<TYPE>();
      static size_t const NUM_ADJ_VERTS_PER_VERT = tilesPerVertex<TYPE>();
    };

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy2<PlanarTileType::QUAD>::adjacentTiles(const glm::ivec2& tile) const {
      return {{ right(tile), up(tile), left(tile), down(tile) }};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy2<PlanarTileType::TRI>::adjacentTiles(const glm::ivec2& tile) const {
      if (tile.y % 2 == 0) {
        return {{ down(tile), upright(tile), up(tile) }};
      } else {
        return {{ downleft(tile), down(tile), up(tile) }};
      }
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy2<PlanarTileType::HEX>::adjacentTiles(const glm::ivec2& tile) const {
      return { {right(tile), up(tile), upleft(tile), left(tile), down(tile), downright(tile) }};
    }


    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy2<PlanarTileType::QUAD>::adjacentVertices(const glm::ivec2& tile) const {
      return {{ tile, right(tile), upright(tile), up(tile) }};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy2<PlanarTileType::TRI>::adjacentVertices(const glm::ivec2& tile) const {
      if (tile.y % 2 == 0) {
        glm::ivec2 t(tile.x, tile.y / 2);
        return {{ t, right(t), upright(t) }};
      } else {
        glm::ivec2 t(tile.x, tile.y / 2 + 1);
        return {{ t, down(t), right(t) }};
      }
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy2<PlanarTileType::HEX>::adjacentVertices(const glm::ivec2& tile) const {
      glm::ivec2 t(2*tile.x+tile.y, tile.x + 2*tile.y);
      return {{ t, right(t), glm::ivec2(t.x+2, t.y+1),
        glm::ivec2(t.x+2, t.y+2), glm::ivec2(t.x+1, t.y+2), up(t) }};
    }


    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::QUAD>()>
    TileTopologyPolicy2<PlanarTileType::QUAD>::edges(const glm::ivec2& tile) const {
      return {{ glm::ivec2(1, 2), glm::ivec2(2, 3), glm::ivec2(3, 0), glm::ivec2(0, 1) }};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::TRI>()>
    TileTopologyPolicy2<PlanarTileType::TRI>::edges(const glm::ivec2& tile) const {
      return {{ glm::ivec2(0, 1), glm::ivec2(1, 2), glm::ivec2(2, 0) }};
    }

    template<>
    std::array<glm::ivec2, vertsPerTile<PlanarTileType::HEX>()>
    TileTopologyPolicy2<PlanarTileType::HEX>::edges(const glm::ivec2& tile) const {
      return {{ glm::ivec2(2, 3), glm::ivec2(3, 4),
                glm::ivec2(4, 5), glm::ivec2(5, 0),
                glm::ivec2(0, 1), glm::ivec2(1, 2)}};
    }


    struct GaussianCoords {
      static glm::vec2 coords(glm::ivec2 tileCoords) {
        return glm::vec2(tileCoords);
      }
    };

    struct EulerIntCoords {
      static glm::vec2 coords(glm::ivec2 tileCoords) {
        glm::vec2 ret = glm::vec2(1.0, 1.0) * glm::vec2(tileCoords);
        return glm::vec2(ret.x + ret.y * -0.5, ret.y * sqrt(3.0));
      }
    };



    template<class T_TYPE, class V_TYPE, class TOPOLOGY, class COORDS = GaussianCoords>
    class PlanarTileMap : private TOPOLOGY, COORDS {
    public:
      struct Vertex;
      struct Tile;
      struct Edge;

      struct Tile {
        glm::ivec2 id;
        size_t adjacentTileSize = 0;

        size_t numAdjacentTiles() const {
          return adjacentTileSize;
        }

        size_t numEdges() const {
          return adjacentTileSize;
        }

        glm::ivec2 tileId() const {
          return id;
        }

        glm::vec2 coords2d() {
          return COORDS::coords(id);
        }

        std::array<Tile*, TOPOLOGY::NUM_ADJ_TILES_PER_TILE> adjacentTiles;
        std::array<Vertex*, TOPOLOGY::NUM_ADJ_VERTS_PER_TILE> adjacentVertices;
        //std::array<std::pair<Tile*, std::pair<Vertex*, Vertex*>>, TOPOLOGY::NUM_ADJ_VERTS_PER_TILE> edges;
        std::array<Edge, TOPOLOGY::NUM_ADJ_VERTS_PER_TILE> edges;

        typedef typename decltype(adjacentVertices)::iterator vertex_iterator;
        typedef typename decltype(adjacentVertices)::const_iterator const_vertex_iterator;

        typedef typename decltype(adjacentTiles)::iterator tile_iterator;
        typedef typename decltype(adjacentTiles)::const_iterator const_tile_iterator;

        typedef typename decltype(edges)::iterator edge_iterator;
        typedef typename decltype(edges)::const_iterator const_edge_iterator;

        tile_iterator tiles_begin() { return adjacentTiles.begin(); }
        tile_iterator tiles_end() { return adjacentTiles.begin() + adjacentTileSize; }
        const_tile_iterator tiles_begin() const noexcept { return adjacentTiles.begin(); }
        const_tile_iterator tiles_end() const noexcept { return adjacentTiles.begin() + adjacentTileSize; }

        vertex_iterator vertices_begin() { return adjacentVertices.begin(); }
        vertex_iterator vertices_end() { return adjacentVertices.end(); }
        const_vertex_iterator vertices_begin() const noexcept { return adjacentVertices.begin(); }
        const_vertex_iterator vertices_end() const noexcept { return adjacentVertices.end(); }

        edge_iterator edges_begin() { return edges.begin(); }
        edge_iterator edges_end() { return edges.end(); }
        const_edge_iterator edges_begin() const noexcept { return edges.begin(); }
        const_edge_iterator edges_end() const noexcept { return edges.end(); }

        T_TYPE data;
      };

      // An edge has 2 vertices and is adjacent to 2 tiless
      struct Edge {
    	  Tile* adjacentTile;
    	  Vertex* v1;
		  Vertex* v2;
      };

      struct Vertex {
        glm::ivec2 id;
        size_t adjacentTileSize = 0;

        size_t numAdjacentTiles() const {
          return adjacentTileSize;
        }

        glm::ivec2 vertexId() const {
          return id;
        }

        glm::vec2 coords2d() {
          return COORDS::coords(id);
        }

        std::array<Tile*, TOPOLOGY::NUM_ADJ_TILES_PER_VERT> adjacentTiles;

        typedef typename decltype(adjacentTiles)::iterator tile_iterator;
        typedef typename decltype(adjacentTiles)::const_iterator const_tile_iterator;

        tile_iterator tiles_begin() { return adjacentTiles.begin(); }
        tile_iterator tiles_end() { return adjacentTiles.begin() + adjacentTileSize; }
        const_tile_iterator tiles_begin() const noexcept { return adjacentTiles.begin(); }
        const_tile_iterator tiles_end() const noexcept { return adjacentTiles.begin() + adjacentTileSize; }

        V_TYPE data;
      };

    private:
      std::unordered_map<glm::ivec2, Tile> tiles;
      std::unordered_map<glm::ivec2, Vertex> verts;

      Tile* findTilesDFS(const glm::ivec2 &tile, const std::function<bool(const glm::ivec2&)> &pred) {
        // Insert the new tile into the tile map
        Tile* tileData = &tiles[tile];
        tileData->id = tile;

        auto adjacentVertIds = this->adjacentVertices(tile);
        auto adjacentTileIds = this->adjacentTiles(tile);
        auto adjacentEdgeIds = this->edges(tile);

        size_t numEdgesInserted = 0;
        // Insert adjacent tiles into tile map and update Tile data structure
        for(unsigned i = 0;i < adjacentTileIds.size(); i++) {
          glm::ivec2 id = adjacentTileIds[i];
          Vertex* v1 = &verts[adjacentVertIds[adjacentEdgeIds[i].x]];
          Vertex* v2 = &verts[adjacentVertIds[adjacentEdgeIds[i].y]];

          auto tile = tiles.find(id);
          if(tile == tiles.end()) {
            if(pred(id)) {
              Tile* t =  findTilesDFS(id, pred);
              tileData->edges[numEdgesInserted++] = Edge{ t, v1, v2 };//std::make_pair(t, std::make_pair(v1, v2));
              tileData->adjacentTiles[tileData->adjacentTileSize++] = t;
            } else {
              tileData->edges[numEdgesInserted++] = Edge{ nullptr, v1, v2 };
            }
          } else {
            Tile* t = &tile->second;
            tileData->edges[numEdgesInserted++] = Edge{ t, v1, v2 }; //std::make_pair(t, std::make_pair(v1, v2));
            tileData->adjacentTiles[tileData->adjacentTileSize++] = t;
          }
        }

        // Insert adjacent vertices for tile into vertex map
        {
          size_t numVerticesInserted = 0;
          for(auto i = adjacentVertIds.begin(); i != adjacentVertIds.end(); i++) {
            Vertex* v1 = &verts[*i];
            v1->id = *i;

            tileData->adjacentVertices[numVerticesInserted++] = v1;
            v1->adjacentTiles[v1->adjacentTileSize++] = tileData;
          }
        }

        return tileData;
      }

    public:
      static glm::vec2 coords2d(const glm::ivec2& v) {
        return COORDS::coords(v);
      }

      typedef typename decltype(tiles)::iterator tile_iterator;
      typedef typename decltype(verts)::iterator vertex_iterator;

      typedef typename decltype(tiles)::const_iterator const_tile_iterator;
      typedef typename decltype(verts)::const_iterator const_vertex_iterator;

      tile_iterator tiles_begin() { return tiles.begin(); }
      tile_iterator tiles_end() { return tiles.end(); }
      const_tile_iterator tiles_begin() const noexcept { return tiles.begin(); }
      const_tile_iterator tiles_end() const noexcept { return tiles.end(); }

      vertex_iterator vertices_begin() { return verts.begin(); }
      vertex_iterator vertices_end() { return verts.end(); }
      const_vertex_iterator vertices_begin() const noexcept { return verts.begin(); }
      const_vertex_iterator vertices_end() const noexcept { return verts.end(); }

      size_t numVertsPerTile() const { return TOPOLOGY::NUM_ADJ_VERTS_PER_TILE; };

      size_t numEdgesPerTile() const { return TOPOLOGY::NUM_ADJ_TILES_PER_TILE; };

      Tile* addTilesInNeighborhood(glm::ivec2 point, const std::function<bool(const glm::ivec2&)> &pred) {
        if (pred(point)) {
          Tile* ret = findTilesDFS(point, pred);
          return ret;
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
    using PlanarTileSet = PlanarTileMap<EmptyStruct, EmptyStruct, TileTopologyPolicy2<T>>;
  }

  using QuadPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::QUAD>;

  using TriPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::TRI>;

  using HexPlanarTileSet = detail::PlanarTileSet<detail::PlanarTileType::HEX>;

  template <typename VT>
  using QuadPlanarTileMapV = detail::PlanarTileMap<detail::EmptyStruct, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::QUAD>>;

  template <typename VT>
  using TriPlanarTileMapV = detail::PlanarTileMap<detail::EmptyStruct, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::TRI>, detail::EulerIntCoords>;

  template <typename VT>
  using HexPlanarTileMapV = detail::PlanarTileMap<detail::EmptyStruct, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::HEX>, detail::EulerIntCoords>;


  template <typename TT>
  using QuadPlanarTileMapT = detail::PlanarTileMap<TT, detail::EmptyStruct, detail::TileTopologyPolicy2<detail::PlanarTileType::QUAD>>;

  template <typename TT>
  using TriPlanarTileMapT = detail::PlanarTileMap<TT, detail::EmptyStruct, detail::TileTopologyPolicy2<detail::PlanarTileType::TRI>, detail::EulerIntCoords>;

  template <typename TT>
  using HexPlanarTileMapT = detail::PlanarTileMap<TT, detail::EmptyStruct, detail::TileTopologyPolicy2<detail::PlanarTileType::HEX>, detail::EulerIntCoords>;


  template <typename TT, typename VT>
  using QuadPlanarTileMapTV = detail::PlanarTileMap<TT, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::QUAD>>;

  template <typename TT, typename VT>
  using TriPlanarTileMapTV = detail::PlanarTileMap<TT, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::TRI>, detail::EulerIntCoords>;

  template <typename TT, typename VT>
  using HexPlanarTileMapTV = detail::PlanarTileMap<TT, VT, detail::TileTopologyPolicy2<detail::PlanarTileType::HEX>, detail::EulerIntCoords>;
}

#endif /* GEOMETRY_PLANAR_TILING_H_ */
