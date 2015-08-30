#ifdef IS_IDE
#error "is ide defined in real build"
#include "tile_mesh.h"
namespace geometry {
#endif

template <class Tiling>
TileMesh<Tiling>::TileMesh(size_t radius) {
  rebuildMesh(radius);
}

template <class Tiling>
TileMesh<Tiling>::~TileMesh() {}


template <class Tiling>
void TileMesh<Tiling>::rebuildMesh(size_t radius, const glm::ivec2& ctr) {
  mTiling.clear();
  auto nearestN = [radius] (const glm::ivec2& tile, const glm::ivec2& ctr) {
    return (distance(Tiling::coords2d(tile), glm::vec2(ctr)) <= radius);
  };

  mTiling.addTilesInNeighborhood(ctr, std::bind(nearestN, std::placeholders::_1, ctr));

  mRebuildGeometry = true;
}

template <class Tiling>
void TileMesh<Tiling>::depthsort(Vertex* verts, GLuint* inds, size_t numIndices) { // Depth sort the triangles
  std::vector<size_t> v;
  v.reserve(numIndices/3);
  for(size_t i = 0; i < numIndices / 3; i++) {
    v.push_back(i);
  }

  auto triDepthCmpFunc = [&](size_t i1, size_t i2) {
    const glm::vec4 c1 = (verts[inds[i1*3]].get<POS>() + verts[inds[i1*3+1]].get<POS>() + verts[inds[i1*3+2]].get<POS>()) / 3.0f;
    const glm::vec4 c2 = (verts[inds[i2*3]].get<POS>() + verts[inds[i2*3+1]].get<POS>() + verts[inds[i2*3+2]].get<POS>()) / 3.0f;
    return glm::distance(glm::vec3(0.0), glm::vec3(c1)) > glm::distance(glm::vec3(0.0), glm::vec3(c2));
  };

  sort(v.begin(), v.end(), triDepthCmpFunc);

  std::vector<GLuint> inds2;
  inds2.reserve(numIndices);
  for(auto i = v.begin(); i != v.end(); i++) {
    GLuint iBase = *i;
    inds2.push_back(inds[iBase * 3]);
    inds2.push_back(inds[iBase * 3 + 1]);
    inds2.push_back(inds[iBase * 3 + 2]);
  }

  memcpy(inds, inds2.data(), numIndices*sizeof(GLuint));
}

template <class Tiling>
GLuint TileMesh<Tiling>::makeTextureArray(size_t w, size_t h, size_t n) {
  GLuint texArray;
  glGenTextures(1, &texArray);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, w, h, n);

  return texArray;
}

template <class Tiling>
void TileMesh<Tiling>::loadImgToTexArray(const std::string& key, GLuint tex, size_t arrayIndex) {
  // Load the image into memory
  int w, h, channels;
  unsigned char* img = SOIL_load_image(key.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

  if(img == 0) {
    throw std::runtime_error(std::string("Failed to load texture: ") + key);
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
      0, // Mipmap level
      0, 0, arrayIndex, // x-offset, y-offset, z-offset
      w, h, 1, // width, height, depth
      GL_RGBA, GL_UNSIGNED_BYTE, img);
  SOIL_free_image_data(img);
}

template <class Tiling>
std::pair<std::string, std::string> TileMesh<Tiling>::getTexKey(const glm::ivec2& tileIndex, const glm::ivec2& adjacentTileIndex) {
  std::string view = "";
  glm::ivec2 diff = adjacentTileIndex - tileIndex; // Used to determine which wall is being looked through
  glm::ivec2 pos = adjacentTileIndex; // Used to determine which tile we are looking at

  if(diff == glm::ivec2(0, -1)) {
    view = "FrontWallView";
  } else if(diff == glm::ivec2(0, 1)) {
    view = "BackWallView";
  } else if(diff == glm::ivec2(1, 0)) {
    view = "LeftWallView";
  } else if(diff == glm::ivec2(-1, 0)) {
    view = "RightWallView";
  } else {
    throw std::runtime_error("Invalid values for getKey, tileIndex and adjacentTileIndex should differ by one in only one dimension.");
  }

  std::string key = view + std::string("_") +
      std::to_string(pos.x) + std::string("_") +
      std::to_string(pos.y) + std::string(".png");

  return make_pair(view, key);
}


// TODO: Identify each face based on the tile it is looking into
template <typename Tiling>
Geometry TileMesh<Tiling>::generateTileGeometry() {

  // These numbers are an overestimate of the actual number of vertices.
  // TODO: Compute exact values
  const size_t numVertices = mTiling.tileCount() * mTiling.numVertsPerTile() * 4;
  const size_t numIndices = mTiling.tileCount() * mTiling.numEdgesPerTile() * 6;

  // Setup the geometry to return
  Geometry ret = Geometry::makeGeometry<Vertex>(numVertices, numIndices);
  glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
  Vertex* verts = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
  GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

  // Maps tiles to unique integer identifiers for those tiles
  std::unordered_map<glm::ivec2, size_t> tileIds;

  // The next tile ID to assign to a new tile
  size_t nextId = 1;

  size_t currentTexIndex = 0;
  std::unordered_map<std::string, size_t> textures;

  mTileTextureArray = makeTextureArray(IMG_DIM, IMG_DIM, mTiling.edgeCount()*2);
  mTileDepthTextureArray = makeTextureArray(IMG_DIM, IMG_DIM, mTiling.edgeCount()*2);

  size_t vOffset = 0, iOffset = 0;
  size_t edgeIndex = 0;
  for(auto t = mTiling.tiles_begin(); t != mTiling.tiles_end(); t++) { // For each tile, t
    for(auto e = t->second.edges_begin(); e != t->second.edges_end(); e++) { // For each edge of t, e
      // The coordinates of the vertices of the e
      const glm::vec2 TILE_CENTROID_OFFSET = Tiling::tileCenterCoords2d(glm::ivec2(0));
      const glm::vec2 v1 = e->v1->coords2d() - TILE_CENTROID_OFFSET;
      const glm::vec2 v2 = e->v2->coords2d() - TILE_CENTROID_OFFSET;
      const size_t vBase = vOffset;

      size_t textureOffset = 0;
      if(e->adjacentTile != nullptr) { // If there is a tile adjacent to i, through e
        typename Tiling::Tile* adjTile = e->adjacentTile; // The tile adjacent to i, through e

        { // Don't load views that are not visible from the center tile
          const glm::vec3 v1_to_v2 = glm::vec3(v2.x, 0.0, v2.y) - glm::vec3(v1.x, 0.0, v1.y);
          const glm::vec3 tangent = normalize(v1_to_v2);
          const glm::vec3 normal = normalize(cross(glm::vec3(0.0, 1.0, 0.0), tangent));
          const glm::vec3 view_dir = normalize((glm::vec3(v1.x, 0.0f, v1.y) + (v1_to_v2 / 2.0f)));
          if(dot(view_dir, normal) < 0.0) {
            continue;
          }
        }

        // Determine the name of the texture to load for the current tile
        std::pair<std::string, std::string> viewName = getTexKey(t->first, adjTile->id);
        std::string key = viewName.second;
        std::string db_key = std::string("textures/db_") + key;
        key = std::string("textures/") + key;

        // Load the appropriate textures determined by the names
        if(textures.find(key) == textures.end()) {
          loadImgToTexArray(key, mTileTextureArray, currentTexIndex);

          textures[key] = currentTexIndex;
          textureOffset = currentTexIndex;

          loadImgToTexArray(db_key, mTileDepthTextureArray, currentTexIndex);

          currentTexIndex += 1;
        } else {
          throw std::runtime_error(
              std::string("Using texture ") + key +
              std::string("which is already being used!"));
        }


        float id = 0; // The id of the tile we are looking into

        glm::vec3 pos = glm::vec3(t->first.x, 0.0, t->first.y);
        if(viewName.first == "FrontWallView") {
          pos += glm::vec3(0.0, 0.0, -0.5);
        } else if(viewName.first == "BackWallView") {
          pos += glm::vec3(0.0, 0.0, 0.5);
        } else if(viewName.first == "LeftWallView") {
          pos += glm::vec3(0.5, 0.0, 0.0);
        } else if(viewName.first == "RightWallView") {
          pos += glm::vec3(-0.5, 0.0,  0.0);
        }

//        std::cout << "distance to " << viewName.first
//                  << " of " << glm::to_string(adjTile->id)
//                  << " is length(" << glm::to_string(pos) << ") = "
//                  << glm::length(pos) << std::endl;

        // Get the id for a tile we've already seen or create a new one for a new tile
        if(tileIds.find(adjTile->id) == tileIds.end()) {
          tileIds[adjTile->id] = nextId;
          id = nextId;
          nextId += 1;
        } else {
          id = tileIds[adjTile->id];
        }
        id = id / (mTiling.tileCount() + 1);


        std::cout << "insert " << glm::to_string(glm::vec3(t->first, edgeIndex)) << " -> " << textureOffset << std::endl;
        viewToTexIndex[glm::vec3(t->first, edgeIndex)] = textureOffset;
        edgeIndex = (edgeIndex + 1) % Tiling::numEdgesPerTile();

        verts[vOffset++] = Vertex{glm::vec4(v1.x,  0.5, v1.y, 1.0), glm::vec3(0.0, 1.0, textureOffset), pos};
        verts[vOffset++] = Vertex{glm::vec4(v1.x, -0.5, v1.y, 1.0), glm::vec3(0.0, 0.0, textureOffset), pos};
        verts[vOffset++] = Vertex{glm::vec4(v2.x,  0.5, v2.y, 1.0), glm::vec3(1.0, 1.0, textureOffset), pos};
        verts[vOffset++] = Vertex{glm::vec4(v2.x, -0.5, v2.y, 1.0), glm::vec3(1.0, 0.0, textureOffset), pos};

        inds[iOffset++] = vBase + 0;
        inds[iOffset++] = vBase + 1;
        inds[iOffset++] = vBase + 2;
        inds[iOffset++] = vBase + 1;
        inds[iOffset++] = vBase + 3;
        inds[iOffset++] = vBase + 2;
      }
    }
  }

  // TODO: This is kind of a hack, our buffers will be too big
  ret.num_indices = iOffset;
  ret.num_vertices = vOffset;

  depthsort(verts, inds, ret.num_indices);

  glUnmapBuffer(GL_ARRAY_BUFFER);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  mNumTextures = currentTexIndex;

  return ret;
}

template <typename Tiling>
void TileMesh<Tiling>::printTextureNames() {
  for(auto t = mTiling.tiles_begin(); t != mTiling.tiles_end(); t++) { // For each tile, t
    for(auto e = t->second.edges_begin(); e != t->second.edges_end(); e++) { // For each edge of t, e
      // The coordinates of the vertices of the e
      const glm::vec2 TILE_CENTROID_OFFSET = Tiling::tileCenterCoords2d(glm::ivec2(0));
      const glm::vec2 v1 = e->v1->coords2d() - TILE_CENTROID_OFFSET;
      const glm::vec2 v2 = e->v2->coords2d() - TILE_CENTROID_OFFSET;

      if(e->adjacentTile != nullptr) { // If there is a tile adjacent to i, through e
        typename Tiling::Tile* adjTile = e->adjacentTile; // The tile adjacent to i, through e

        { // Don't load views that are not visible from the center tile
          const glm::vec3 v1_to_v2 = glm::vec3(v2.x, 0.0, v2.y) - glm::vec3(v1.x, 0.0, v1.y);
          const glm::vec3 tangent = normalize(v1_to_v2);
          const glm::vec3 normal = normalize(cross(glm::vec3(0.0, 1.0, 0.0), tangent));
          const glm::vec3 view_dir = normalize((glm::vec3(v1.x, 0.0f, v1.y) + (v1_to_v2 / 2.0f)));
          if(dot(view_dir, normal) < 0.0) {
            continue;
          }
        }

        // Determine the name of the texture to load for the current tile
        std::pair<std::string, std::string> viewName = getTexKey(t->first, adjTile->id);

        // Print the texture name
        std::cout << viewName.first << "," << std::to_string(adjTile->id.x) << "," << std::to_string(adjTile->id.y) << std::endl;
      }
    }
  }
}

#ifdef IS_IDE
}
#endif
