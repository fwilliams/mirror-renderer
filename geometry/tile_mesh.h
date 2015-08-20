#include <GL/glew.h>
#include <SOIL/SOIL.h>

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

#include "geometry/planar_tiling.h"
#include "geometry/3d_primitives.h"

#ifndef TILEMESH_H_
#define TILEMESH_H_

namespace geometry {

enum Mode {
  TEXTURED,
  IDENTIFIED
};

template <geometry::Mode mode, class Tiling>
class TileMesh {
public:
	struct Vertex;
private:
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

	Geometry generateTexturedTileGeometry();

	Geometry generateIdentifiedTileGeometry();

public:
	void printTextureNames();

	struct Vertex {
		glm::vec4 pos;
		glm::vec3 texcoord;
	};

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
	    switch(mode) {
	    case TEXTURED:
        mGeometry = generateTexturedTileGeometry();
        break;
	    case IDENTIFIED:
	      mGeometry = generateIdentifiedTileGeometry();
	      break;
	    }
	    mRebuildGeometry = false;
	  }
		return mGeometry;
	}

	void rebuildMesh(size_t radius);

	TileMesh(size_t radius);

	virtual ~TileMesh();
};

#ifndef IS_IDE
#include "tile_mesh.inl.cpp"
#endif

}
#endif /* TILEMESH_H_ */
