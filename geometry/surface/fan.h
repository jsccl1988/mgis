#ifndef GEOMETRY_POINT_H
#define GEOMETRY_POINT_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Fan : public Surface {
 public:
  Fan();
  virtual ~Fan();

  // Geometry interface
  virtual int GetDimension() const;
  virtual GeometryType GetGeometryType() const;
  virtual const char *GetGeometryName() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual void GetEnvelope(Envelope *envelope) const;
  virtual bool IsEmpty() const;

  // Surface Interface
  virtual double GetArea() const;
  virtual int Centroid(Point *poPoint) const;
  virtual int PointOnSurface(Point *poPoint) const;

  // Fan methods
  Arc *GetArc(void) { return m_pArc; }
  const Arc *GetArc(void) const { return m_pArc; }

  void SetArc(Arc *pArc);
  void SetArcDirectly(Arc *pArc);

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  // non standard.
  virtual void SetCoordinateDimension(int nDimension);

 protected:
  Arc *m_pArc;
};
}  // namespace core
#endif  // GEOMETRY_POINT_H