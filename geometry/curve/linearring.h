#ifndef GEOMETRY_CURVE_LINEARRING_H
#define GEOMETRY_CURVE_LINEARRING_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT LinearRing : public Spline {
 public:
  friend class Polygon;

  LinearRing();
  LinearRing(LinearRing *);
  virtual ~LinearRing();

  // Non standard.
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;
  virtual Geometry *Clone() const;
  virtual bool IsClockwise() const;
  virtual void ReverseWindingOrder();
  virtual void CloseRings();
  virtual double GetArea() const;
  bool IsPointInRing(const Point *pt, bool bTestEnvelope = true) const;
  bool IsPointOnRingBoundary(const Point *pt, bool bTestEnvelope = true) const;
};
}  // namespace core
#endif  // GEOMETRY_CURVE_LINEARRING_H