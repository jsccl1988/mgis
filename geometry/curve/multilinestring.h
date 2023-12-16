#ifndef GEOMETRY_CURVE_MULTILINESTRING_H
#define GEOMETRY_CURVE_MULTILINESTRING_H

#include "geometry/geometry.h"

namespace core {
class SMT_EXPORT_CLASS MultiLineString : public GeometryCollection {
 public:
  MultiLineString();
  virtual ~MultiLineString();

  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual SmtGeometryType GetGeometryType() const;
  virtual SmtGeometry *Clone() const;

  // Non standard
  virtual int AddGeometryDirectly(SmtGeometry *);
};
}  // namespace core
#endif  // GEOMETRY_CURVE_MULTILINESTRING_H