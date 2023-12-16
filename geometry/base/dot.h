#ifndef GEOMETRY_BASE_DOT_H
#define GEOMETRY_BASE_DOT_H

#include "core/core.h"
#include "geometry/export.h"

namespace core {
template <typename T>
struct Dot2D {
  T x, y;

  bool operator==(const Dot2D<T>& rhs) const {
    return (x == rhs.x && y == rhs.y);
  }

  bool operator!=(const Dot2D<T>& rhs) const { return !(*this == lpt); }
};

template <typename T>
struct Dot3D {
  T x, y, z;

  bool operator==(const Dot3D<T>& rhs) const {
    return (x == rhs.x && y == rhs.y && z == rhs.z);
  }

  bool operator!=(const Dot3D<T>& rhs) const { return !(*this == lpt); }
};
}  // namespace core
#endif  // GEOMETRY_BASE_DOT_H