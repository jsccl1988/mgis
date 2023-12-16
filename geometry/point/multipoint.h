#ifndef GEOMETRY_POINT_MULTIPOINT_H
#define GEOMETRY_POINT_MULTIPOINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT MultiPoint : public GeometryCollection {
 public:
  MultiPoint();
  virtual ~MultiPoint();

  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;
  virtual Geometry *Clone() const;

  virtual void GetEnvelope(Envelope *envelope) const;

  // Non standard
  virtual int AddGeometryDirectly(Geometry *);

  // Spatial Relationship
  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;
};
}  // namespace core
#endif  // GEOMETRY_POINT_MULTIPOINT_H