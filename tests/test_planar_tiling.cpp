#define BOOST_TEST_DYN_LINK

#include <boost/test/unit_test.hpp>

#include "geometry/planar_tiling.h"

using namespace glm;
using namespace std;
using namespace geometry;

struct PlanarTileSetFixture {
  function<bool(const ivec2&)> isInGrid = [](const ivec2& v) {
    for(unsigned i = 0; i < 3; i++) {
      for(unsigned j = 0; j < 3; j++) {
        if(v == ivec2(i, j)) {
          return true;
        }
      }
    }
    return false;
  };

  PlanarTileSetFixture() {
  }

  ~PlanarTileSetFixture() {
  }
};

using namespace glm;

BOOST_FIXTURE_TEST_SUITE(PlanarTileSetTests, PlanarTileSetFixture)

BOOST_AUTO_TEST_CASE(test_trivial_quad) {
  QuadPlanarTileSet testTileset;
  auto derp = [](const glm::ivec2& v) { return v == glm::ivec2(0, 0); };
  testTileset.addTilesInNeighborhood(ivec2(0), derp);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 1);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 4);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 4);
}

BOOST_AUTO_TEST_CASE(test_trivial_tri) {
  TriPlanarTileSet testTileset;
  auto derp = [](const glm::ivec2& v) { return v == glm::ivec2(0, 0); };
  testTileset.addTilesInNeighborhood(ivec2(0), derp);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 1);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 3);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 3);
}

BOOST_AUTO_TEST_CASE(test_trivial_hex) {
  HexPlanarTileSet testTileset;
  auto derp = [](const glm::ivec2& v) { return v == glm::ivec2(0, 0); };
  testTileset.addTilesInNeighborhood(ivec2(0), derp);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 1);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 6);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 6);
}

BOOST_AUTO_TEST_CASE(test_grid_of_quad_tiles) {
  QuadPlanarTileSet testTileset;
  QuadPlanarTileSet::Tile* t = testTileset.addTilesInNeighborhood(ivec2(0), isInGrid);
//  BOOST_CHECK_EQUAL(t->tileId(), glm::ivec2(0));
//  BOOST_CHECK_EQUAL(t->numAdjacentTiles(), 4);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 9);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 16);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 24);

}

BOOST_AUTO_TEST_CASE(test_grid_of_tri_tiles) {
  TriPlanarTileSet testTileset;
  TriPlanarTileSet::Tile* t = testTileset.addTilesInNeighborhood(ivec2(0), isInGrid);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 9);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 11);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 19);
}

BOOST_AUTO_TEST_CASE(test_tri_immediate_neighbors) {
  auto pred = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 0), ivec2(0, -1), ivec2(1, 1) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) { return true; }
    }
    return false;
  };

  TriPlanarTileSet tps;
  tps.addTilesInNeighborhood(ivec2(0, 0), pred);
  BOOST_CHECK_EQUAL(tps.edgeCount(), 9);
  BOOST_CHECK_EQUAL(tps.vertexCount(), 6);
  BOOST_CHECK_EQUAL(tps.tileCount(), 4);
}

BOOST_AUTO_TEST_CASE(test_quad_immediate_neighbors) {
  auto pred = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 0), ivec2(0, -1), ivec2(-1, 0), ivec2(0, 1) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) { return true; }
    }
    return false;
  };

  QuadPlanarTileSet tps;
  tps.addTilesInNeighborhood(ivec2(0, 0), pred);
  BOOST_CHECK_EQUAL(tps.edgeCount(), 16);
  BOOST_CHECK_EQUAL(tps.vertexCount(), 12);
  BOOST_CHECK_EQUAL(tps.tileCount(), 5);
}

BOOST_AUTO_TEST_CASE(test_hex_immediate_neighbors) {
  auto pred = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 0), ivec2(-1, 0),
               ivec2(0, 1), ivec2(0, -1), ivec2(1, -1), ivec2(-1, 1) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) { return true; }
    }
    return false;
  };

  HexPlanarTileSet tps;
  tps.addTilesInNeighborhood(ivec2(0, 0), pred);
  BOOST_CHECK_EQUAL(tps.edgeCount(), 30);
  BOOST_CHECK_EQUAL(tps.vertexCount(), 24);
  BOOST_CHECK_EQUAL(tps.tileCount(), 7);
}

BOOST_AUTO_TEST_CASE(test_small_hex_neighborhood) {
  auto pred = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 0), ivec2(-1, 0),
               ivec2(0, 1), ivec2(0, -1), ivec2(1, -1), ivec2(-1, -1) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) { return true; }
    }
    return false;
  };

  HexPlanarTileSet tps;
  tps.addTilesInNeighborhood(ivec2(0, 0), pred);
  BOOST_CHECK_EQUAL(tps.edgeCount(), 31);
  BOOST_CHECK_EQUAL(tps.vertexCount(), 25);
  BOOST_CHECK_EQUAL(tps.tileCount(), 7);
}

BOOST_AUTO_TEST_CASE(test_hourglass_tri_tiles_only_captures_tri_in_neighborhood) {
  auto hourGlass = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 3) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) { return true; }
    }
    return false;
  };

  TriPlanarTileSet testTileset;
  testTileset.addTilesInNeighborhood(ivec2(0), hourGlass);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 3);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 3);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 1);
}

BOOST_AUTO_TEST_CASE(test_quads_with_one_vertex_overlap) {
  auto pred = [] (const ivec2& v) {
    auto c = { ivec2(0, 0), ivec2(1, 1) };
    for(auto i = c.begin(); i != c.end(); i++) {
      if(*i == v) {
        return true;
      }
    }
    return false;
  };

  QuadPlanarTileSet testTileset;
  testTileset.addTilesInNeighborhood(ivec2(0), pred);
  BOOST_CHECK_EQUAL(testTileset.edgeCount(), 4);
  BOOST_CHECK_EQUAL(testTileset.vertexCount(), 4);
  BOOST_CHECK_EQUAL(testTileset.tileCount(), 1);
}


BOOST_AUTO_TEST_SUITE_END()
