#ifndef GEOMETRY_POINT_H
#define GEOMETRY_POINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Surface : public Geometry {
 public:
  virtual double GetArea() const = 0;
  virtual int Centroid(Point *poPoint) const = 0;
  virtual int PointOnSurface(Point *poPoint) const = 0;
};
}  // namespace core
#endif  // GEOMETRY_POINT_H