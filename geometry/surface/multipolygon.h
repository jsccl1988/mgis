#ifndef GEOMETRY_POINT_H
#define GEOMETRY_POINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT MultiPolygon : public GeometryCollection {
 public:
  MultiPolygon();
  virtual ~MultiPolygon();

  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;
  virtual Geometry *Clone() const;

  // Non standard
  virtual int AddGeometryDirectly(Geometry *);

  virtual double GetArea() const;
};
}  // namespace core
#endif  // GEOMETRY_POINT_H