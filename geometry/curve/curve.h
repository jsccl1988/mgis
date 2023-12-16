#ifndef GEOMETRY_CURVE_CURVE_H
#define GEOMETRY_CURVE_CURVE_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Curve : public Geometry {
 public:
  Curve();
  virtual ~Curve();

  // Curve methods
  virtual double GetLength() const = 0;
  virtual void StartPoint(Point *) const = 0;
  virtual void EndPoint(Point *) const = 0;
  virtual bool IsClosed() const;
  virtual void Value(double, Point *) const = 0;
};
}  // namespace core
#endif  // GEOMETRY_CURVE_CURVE_H