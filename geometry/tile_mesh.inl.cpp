using namespace std;
using namespace glm;
using namespace geometry;

template <class Tiling>
TileMesh<Tiling>::TileMesh(size_t radius) {
	mGeometry = generateTileGeometry(mTiling, radius);
}

template <class Tiling>
TileMesh<Tiling>::~TileMesh() {}

template <class Tiling>
void TileMesh<Tiling>::depthsort(Vertex* verts, GLuint* inds, size_t numIndices) {
	vector<size_t> v;
	v.reserve(numIndices/3);
	for(size_t i = 0; i < numIndices / 3; i++) {
		v.push_back(i);
	}

	auto triDepthCmpFunc = [&](size_t i1, size_t i2) {
		const vec4 c1 = (verts[inds[i1*3]].pos + verts[inds[i1*3+1]].pos + verts[inds[i1*3+2]].pos) / 3.0f;
		const vec4 c2 = (verts[inds[i2*3]].pos + verts[inds[i2*3+1]].pos + verts[inds[i2*3+2]].pos) / 3.0f;
		return distance(vec3(0.0), vec3(c1)) > distance(vec3(0.0), vec3(c2));
	};

	sort(v.begin(), v.end(), triDepthCmpFunc);

	vector<GLuint> inds2;
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
void TileMesh<Tiling>::loadImgToTexArray(const string& key, GLuint tex, size_t arrayIndex) {
	// Load the image into memory
	int w, h, channels;
	unsigned char* img = SOIL_load_image(key.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

	if(img == 0) {
		throw runtime_error(string("Failed to load texture: ") + key);
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
pair<string, string> TileMesh<Tiling>::getTexKey(const ivec2& tileIndex, const ivec2& adjacentTileIndex) {
	string view = "";
	ivec2 diff = adjacentTileIndex - tileIndex; // Used to determine which wall is being looked through
	ivec2 pos = adjacentTileIndex; // Used to determine which tile we are looking at

	if(diff == ivec2(0, -1)) {
		view = "FrontWallView";
	} else if(diff == ivec2(0, 1)) {
		view = "BackWallView";
	} else if(diff == ivec2(1, 0)) {
		view = "LeftWallView";
	} else if(diff == ivec2(-1, 0)) {
		view = "RightWallView";
	} else {
		throw runtime_error("Invalid values for getKey, tileIndex and adjacentTileIndex should differ by one in only one dimension.");
	}

	string key = view + string("_") +
			to_string(pos.x) + string("_") +
			to_string(pos.y) + string(".png");

	return make_pair(view, key);
}


// TODO: Identify each face based on the tile it is looking into
template <typename Tiling>
Geometry TileMesh<Tiling>::generateTileGeometry(Tiling& tiling, size_t radius) {
	auto nearestN = [radius] (const glm::ivec2& tile) {
		return distance(Tiling::coords2d(tile), vec2(0)) <= radius;
	};

	tiling.addTilesInNeighborhood(ivec2(0), nearestN);

	const size_t numVertices = tiling.tileCount() * tiling.numVertsPerTile() * 4;
	const size_t numIndices = tiling.tileCount() * tiling.numEdgesPerTile() * 6;

	Geometry ret = Geometry::fromVertexAttribs<vec4, vec3>(numVertices, numIndices);

	glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);

	Vertex* verts = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
	GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));


	const size_t IMG_DIM = 512;
	size_t currentTexIndex = 0;
	unordered_map<string, size_t> textures;

	mTileTextureArray = makeTextureArray(IMG_DIM, IMG_DIM, tiling.edgeCount()*2);
	mTileDepthTextureArray = makeTextureArray(IMG_DIM, IMG_DIM, tiling.edgeCount()*2);

	size_t vOffset = 0, iOffset = 0;
	for(auto t = tiling.tiles_begin(); t != tiling.tiles_end(); t++) { // For each tile, t
		for(auto e = t->second.edges_begin(); e != t->second.edges_end(); e++) { // For each edge of t, e
			// The coordinates of the vertices of the e
			const vec2 v1 = e->v1->coords2d();
			const vec2 v2 = e->v2->coords2d();
			const size_t vBase = vOffset;

			size_t textureOffset = 0;
			if(e->adjacentTile != nullptr) { // If there is a tile adjacent to i, through e
				typename Tiling::Tile* adjTile = e->adjacentTile; // The tile adjacent to i, through e

				{ // Don't load views that are not visible from the center tile
					const vec3 v1_to_v2 = vec3(v2.x, 0.0, v2.y) - vec3(v1.x, 0.0, v1.y);
					const vec3 tangent = normalize(v1_to_v2);
					const vec3 normal = normalize(cross(vec3(0.0, 1.0, 0.0), tangent));
					const vec3 view_dir = normalize((vec3(v1.x, 0.0f, v1.y) + (v1_to_v2 / 2.0f)));
					if(dot(view_dir, normal) < 0.0) {
						continue;
					}
				}

				// Determine the name of the texture to load for the current tile
				pair<string, string> viewName = getTexKey(t->first, adjTile->id);
				string key = viewName.second;
				string db_key = string("textures/db_") + key;
				key = string("textures/") + key;

				//          cout << view << "," << to_string(pos.x) << "," << to_string(pos.y) << endl;

				if(textures.find(key) == textures.end()) {
					loadImgToTexArray(key, mTileTextureArray, currentTexIndex);

					textures[key] = currentTexIndex;
					textureOffset = currentTexIndex;

					loadImgToTexArray(db_key, mTileDepthTextureArray, currentTexIndex);

					currentTexIndex += 1;
				} else {
					throw runtime_error(
							string("Using texture ") + key +
							string("which is already being used!"));
				}


				verts[vOffset++] = {vec4(v1.x,  0.5, v1.y, 1.0), vec3(0.0, 0.0, textureOffset)};
				verts[vOffset++] = {vec4(v1.x, -0.5, v1.y, 1.0), vec3(0.0, 1.0, textureOffset)};
				verts[vOffset++] = {vec4(v2.x,  0.5, v2.y, 1.0), vec3(1.0, 0.0, textureOffset)};
				verts[vOffset++] = {vec4(v2.x, -0.5, v2.y, 1.0), vec3(1.0, 1.0, textureOffset)};

				inds[iOffset++] = vBase + 0;
				inds[iOffset++] = vBase + 1;
				inds[iOffset++] = vBase + 2;
				inds[iOffset++] = vBase + 1;
				inds[iOffset++] = vBase + 3;
				inds[iOffset++] = vBase + 2;
			}
		}
	}


	{ // Depth sort the triangles
		vector<size_t> v;
		v.reserve(numIndices/3);
		for(size_t i = 0; i < numIndices / 3; i++) {
			v.push_back(i);
		}

		auto triDepthCmpFunc = [&](size_t i1, size_t i2) {
			const vec4 c1 = (verts[inds[i1*3]].pos + verts[inds[i1*3+1]].pos + verts[inds[i1*3+2]].pos) / 3.0f;
			const vec4 c2 = (verts[inds[i2*3]].pos + verts[inds[i2*3+1]].pos + verts[inds[i2*3+2]].pos) / 3.0f;
			return distance(vec3(0.0), vec3(c1)) > distance(vec3(0.0), vec3(c2));
		};

		sort(v.begin(), v.end(), triDepthCmpFunc);

		vector<GLuint> inds2;
		inds2.reserve(numIndices);
		for(auto i = v.begin(); i != v.end(); i++) {
			GLuint iBase = *i;
			inds2.push_back(inds[iBase * 3]);
			inds2.push_back(inds[iBase * 3 + 1]);
			inds2.push_back(inds[iBase * 3 + 2]);
		}

		memcpy(inds, inds2.data(), numIndices*sizeof(GLuint));
	}

	glUnmapBuffer(GL_ARRAY_BUFFER);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	mNumTextures = currentTexIndex;

	return ret;
}
