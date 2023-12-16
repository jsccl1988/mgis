#ifndef GEOMETRY_BASE_TILE_H
#define GEOMETRY_BASE_TILE_H

#include "core/core.h"
#include "geometry/base/rect.h"
#include "geometry/export.h"

namespace core {
struct Tile {
  long lID;
  long lTileBufSize;
  char* pTileBuf;
  long lImageCode;
  bool bVisible;

  Rect2D<float> rtTileRect;

  Tile()
      : pTileBuf(NULL),
        lTileBufSize(0),
        lImageCode(-1),
        lID(-1),
        bVisible(true) {}
};

typedef Tile WSTile;

}  // namespace core

#endif  // GEOMETRY_BASE_TILE_H