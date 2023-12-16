#ifndef GEOMETRY_POINT_POINT_H
#define GEOMETRY_POINT_POINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Point : public Geometry {
 public:
  Point();
  Point(double x, double y);
  virtual ~Point();

  // Geometry
  virtual void SetCoordinateDimension(int nDimension);
  virtual int GetDimension() const;
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual void GetEnvelope(Envelope *envelope) const;
  virtual bool IsEmpty() const;

  // Point
  inline double GetX() const { return x; }
  inline double GetY() const { return y; }

  // Non standard
  inline void SetX(double xIn) {
    x = xIn;
    if (coord_dimension_ == 0) coord_dimension_ = 2;
  }
  inline void SetY(double yIn) {
    y = yIn;
    if (coord_dimension_ == 0) coord_dimension_ = 2;
  }

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;
  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;

  //
  virtual double Distance(const Geometry *) const;

 private:
  double x, y;
};
}  // namespace core
#endif  // GEOMETRY_POINT_POINT_H