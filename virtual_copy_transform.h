#include "utils/gl_utils.h"
#include "utils/interactive_window.h"

#include "geometry/tile_mesh.h"

#ifndef VIRTUAL_COPY_TRANSFORM_H_
#define VIRTUAL_COPY_TRANSFORM_H_

using RenderMesh = geometry::TileMesh<geometry::QuadPlanarTileSet>;

class Config {
public:
private:
  std::array<geometry::Quad4, 4> mMirrorFaces = RenderMesh::edgeFaces();

  glm::ivec3 mCurrentView;
public:
  bool showWireFrame = false;

  geometry::Quad4 currentMirror() {
    return mMirrorFaces[mCurrentView.z];
  }

  void nextMirror() {
    mCurrentView.z = (mCurrentView.z + 1) % 4;
  }

  void upView() {
    mCurrentView.y++;
  }

  void downView() {
    mCurrentView.y--;
  }


  glm::ivec3 currentView() {
    return mCurrentView;
  }
};




#endif /* VIRTUAL_COPY_TRANSFORM_H_ */
