#include <iostream>
#include <tuple>
#include <functional>
#include <climits>
#include <type_traits>

#include <glm/glm.hpp>

#include <SOIL/SOIL.h>

#include "utils/gl_utils.h"
#include "utils/sdl_gl_window.h"
#include "renderer/renderer.h"
#include "renderer/camera.h"

#include "geometry/planar_tiling.h"

using namespace std;
using namespace glm;
using namespace geometry;
using namespace renderer;

class ProjectedTileVizWindow: public SDLGLWindow {
  QuadPlanarTileMapV<GLuint> tileMap;

  typedef Renderer<GlVersion::GL330> Rndr;
  Rndr* rndr = nullptr;

  Geometry grid;

  GLuint solidTextureProgram = 0;
  GLuint solidColorProgram = 0;

  GLuint textureArrayId = 0;

  FirstPersonCamera camera;

public:
  ProjectedTileVizWindow(unsigned w, unsigned h) : SDLGLWindow(w, h) {}

  template <typename Tiling>
  Geometry generate2dTileGeometry(Tiling& tiling) {
    auto nearestN = [] (const glm::ivec2& tile) {
      return distance(Tiling::coords2d(tile), vec2(0)) <= 3;
    };

    tiling.addTilesInNeighborhood(ivec2(0), nearestN);

    const size_t numVertices = tiling.vertexCount();
    const size_t numIndices = tiling.tileCount() * tiling.numVertsPerTile() * 2;

    Geometry ret = Geometry::fromVertexAttribs<vec4, vec4>(numVertices, numIndices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    typedef tuple<vec4, vec4> vertex;
    vertex* verts = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));

    size_t vOffset = 0, iOffset = 0;
    for(auto i = tiling.vertices_begin(); i != tiling.vertices_end(); i++) {
      i->second.data = vOffset;
      const vec2 pos = i->second.coords2d();
      verts[vOffset++] = make_tuple(vec4(1.0), vec4(pos.x, 0.0, pos.y, 1.0));
    }

    for(auto i = tiling.tiles_begin(); i != tiling.tiles_end(); i++) {
      for(auto v = i->second.vertices_begin(); v != i->second.vertices_end(); v++) {
        if(v == i->second.vertices_end() - 1) {
          inds[iOffset++] = (*v)->data;
          inds[iOffset++] = (*(i->second.vertices_begin()))->data;
        } else {
          inds[iOffset++] = (*v)->data;
          inds[iOffset++] = (*(v+1))->data;
        }
      }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return ret;
  }

  template <typename UnsignedType>
  UnsignedType roundToNearestPOT(UnsignedType v) {
    static_assert(is_unsigned<UnsignedType>::value, "Only works for unsigned types");
    v--;
    for (size_t i = 1; i < sizeof(v) * CHAR_BIT; i *= 2) {
      v |= v >> i;
    }
    return ++v;
  }

  template <typename Tiling>
  Geometry generate3dTileGeometry(Tiling& tiling) {
    auto nearestN = [] (const glm::ivec2& tile) {
      return distance(Tiling::coords2d(tile), vec2(0)) <= 5;
    };

    tiling.addTilesInNeighborhood(ivec2(0), nearestN);

    const size_t numVertices = tiling.tileCount() * tiling.numVertsPerTile() * 4;
    const size_t numIndices = tiling.tileCount() * tiling.numEdgesPerTile() * 6;

    Geometry ret = Geometry::fromVertexAttribs<vec4, vec3>(numVertices, numIndices);

    glBindBuffer(GL_ARRAY_BUFFER, ret.vbo);
    struct vertex {
      vec4 pos;
      vec3 tex;
    };
    vertex* verts = reinterpret_cast<vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ret.ibo);
    GLuint* inds = reinterpret_cast<GLuint*>(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE));


    const size_t IMG_DIM = 512;
    size_t currentTexIndex = 0;
    unordered_map<string, size_t> textures;

    glActiveTexture(GL_TEXTURE0+7);

    glGenTextures(1, &textureArrayId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, IMG_DIM, IMG_DIM, tiling.edgeCount()*2);

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
        	const vec3 view_dir = normalize((vec3(v1.x, 0.0f, v1.y) + (v1_to_v2 / 2.0f)) - camera.getPosition());
        	if(dot(view_dir, normal) < 0.0) {
        		continue;
        	}
          }

          // Determine the name of the texture to load for the current tile
          string view = "";
          ivec2 diff = adjTile->id - t->first; // Used to determine which wall is being looked through
          ivec2 pos = adjTile->id; // Used to determine which tile we are looking at

          if(diff == ivec2(0, -1)) {
            view = "FrontWallView";
          } else if(diff == ivec2(0, 1)) {
            view = "BackWallView";
          } else if(diff == ivec2(1, 0)) {
            view = "LeftWallView";
          } else if(diff == ivec2(-1, 0)) {
            view = "RightWallView";
          } else {
            throw runtime_error("Got invalid diff value... This should never happen");
          }

          string key =
              string("textures/") + view + string("_") +
              to_string(pos.x) + string("_") +
              to_string(pos.y) + string(".png");

//          cout << view << "," << to_string(pos.x) << "," << to_string(pos.y) << endl;

          if(textures.find(key) == textures.end()) {
            // Load the image into memory
            int w, h, channels;
            unsigned char* img = SOIL_load_image(key.c_str(), &w, &h, &channels, SOIL_LOAD_RGBA);

            if(img == 0) {
              throw runtime_error(string("Failed to load ") + key);
            }

            glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                0, // Mipmap level
                0, 0, currentTexIndex, // x-offset, y-offset, z-offset
                w, h, 1, // width, height, depth
                GL_RGBA, GL_UNSIGNED_BYTE, img);
            SOIL_free_image_data(img);

            textures[key] = currentTexIndex;
            textureOffset = currentTexIndex;
            currentTexIndex += 1;
          } else {
        	  throw runtime_error(
        			  string("Using texture ") + key +
					  string("which is already being used!"));
          }
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

    return ret;
  }

  void setup(SDLGLWindow& w) {
    rndr = new Rndr();
    check_gl_error();
    rndr->setClearColor(vec4(0.1, 0.1, 0.1, 1.0));
    rndr->enableDepthBuffer();
    rndr->enableFaceCulling();
    rndr->enableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    camera.setPosition(vec3(0.5, 0.0, 0.5));
    camera.setPerspectiveProjection(45.0, w.aspectRatio(), 0.1, 10000.0);
    camera.setCameraVelocity(vec2(0.05));

    rndr->addShaderIncludeDir("shaders");
    solidTextureProgram = rndr->makeProgramFromFiles("shaders/solid_texture_vert.glsl",
                                         "shaders/solid_texture_frag.glsl");
    solidColorProgram = rndr->makeProgramFromFiles("shaders/solid_color_vert.glsl",
                                         "shaders/solid_color_frag.glsl");

    grid = generate3dTileGeometry(tileMap);

    setMousePosition(w.width()/2, w.height()/2);
    showCursor(false);
  }

  void handle_event(SDLGLWindow& w, const SDL_Event& event) {
    if(event.type == SDL_KEYDOWN) {
      if(event.key.keysym.sym == SDLK_w) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::POSITIVE);
      }
      if(event.key.keysym.sym == SDLK_s) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
      }
      if(event.key.keysym.sym == SDLK_d) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::POSITIVE);
      }
      if(event.key.keysym.sym == SDLK_a) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::NEGATIVE);
      }
    }

    if(event.type == SDL_KEYUP) {
      if(event.key.keysym.sym == SDLK_w || event.key.keysym.sym == SDLK_s) {
        camera.setForwardDirection(FirstPersonCamera::CameraDirection::STOPPED);
      }
      if(event.key.keysym.sym == SDLK_d || event.key.keysym.sym == SDLK_a) {
        camera.setHorizontalDirection(FirstPersonCamera::CameraDirection::STOPPED);
      }
    }

    if(event.type == SDL_MOUSEBUTTONDOWN) {
      if(event.button.button == SDL_BUTTON_LEFT) {
        cout << to_string(camera.getLookatVector()) << endl;
      }
    }
  }

  void update(SDLGLWindow& w) {
    vec2 mp;
    getNormalizedMousePos(value_ptr(mp));
    setMousePosition(width()/2, height()/2);
    camera.updateLookat(mp);
    camera.updatePosition();
    rndr->setProjectionMatrix(camera.getProjectionMatrix());
    rndr->setViewMatrix(camera.getViewMatrix());
  }

  void draw(SDLGLWindow& w) {
    rndr->clearViewport();
    rndr->startFrame();

    rndr->setProgram(solidTextureProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayId);
    glUniform1i(glGetUniformLocation(solidTextureProgram, "texid"), 0);

    rndr->draw(grid, mat4(1.0), PrimitiveType::TRIANGLES);

    rndr->setProgram(solidColorProgram);
    glUniform4fv(glGetUniformLocation(solidColorProgram, "color"), 1, value_ptr(vec4(0.0, 1.0, 0.0, 1.0)));

    glLineWidth(1.0);
    rndr->disableDepthBuffer();
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    rndr->draw(grid, scale(mat4(1.0), vec3(1.0)), PrimitiveType::TRIANGLES);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    rndr->enableDepthBuffer();
  }
};

int main(int argc, char** argv) {
  ProjectedTileVizWindow w(800, 600);
  w.mainLoop();
}
