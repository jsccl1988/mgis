#ifndef GEOMETRY_POINT_H
#define GEOMETRY_POINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Polygon : public Surface {
 public:
  Polygon();
  virtual ~Polygon();

  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual bool IsEmpty() const;

  // Surface Interface
  virtual double GetArea() const;
  virtual int Centroid(Point *poPoint) const;
  virtual int PointOnSurface(Point *poPoint) const;

  // Geometry
  virtual int GetDimension() const;
  virtual void GetEnvelope(Envelope *envelope) const;

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;

  //
  virtual double Distance(const Geometry *) const;

  // Non standard
  virtual void SetCoordinateDimension(int nDimension);

  void AddRing(LinearRing *);
  void AddRingDirectly(LinearRing *);

  LinearRing *GetExteriorRing();
  const LinearRing *GetExteriorRing() const;

  int GetNumInteriorRings() const;
  LinearRing *GetInteriorRing(int);
  const LinearRing *GetInteriorRing(int) const;

  bool IsPointOnSurface(const Point *) const;

  virtual void CloseRings();

 private:
  int m_nRingCount;
  LinearRing **m_pRings;
};
}  // namespace core
#endif  // GEOMETRY_POINT_H