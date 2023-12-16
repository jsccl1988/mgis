#ifndef GEOMETRY_CURVE_ARC_H
#define GEOMETRY_CURVE_ARC_H

#include "geometry/geometry.h"

namespace core {
class SMT_EXPORT_DLL Arc : public Curve {
 public:
  Arc();
  virtual ~Arc();

  // Geometry interface
  virtual int GetDimension() const;
  virtual GeometryType GetGeometryType() const;
  virtual const char *GetGeometryName() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual void GetEnvelope(Envelope *envelope) const;
  virtual bool IsEmpty() const;

  // Curve methods
  virtual double GetLength() const;
  virtual void StartPoint(Point *) const;
  virtual void EndPoint(Point *) const;
  virtual void Value(double, Point *) const;

  // LineString methods
  float GetRadius(void) const { return m_fRadius; }
  void GetCenterPoint(Point *oPoint) const {
    oPoint->SetX(m_oPointCenter.x);
    oPoint->SetY(m_oPointCenter.y);
  }

  void SetRadius(float radius) { m_fRadius = radius; }
  void SetCenterPoint(RawPoint oPoint) {
    m_oPointCenter.x = oPoint.x;
    m_oPointCenter.y = oPoint.y;
  }
  void SetStartPoint(RawPoint oPoint) {
    m_oPointStart.x = oPoint.x;
    m_oPointStart.y = oPoint.y;
  }
  void SetEndPoint(RawPoint oPoint) {
    m_oPointEnd.x = oPoint.x;
    m_oPointEnd.y = oPoint.y;
  }

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  // non standard.
  virtual void SetCoordinateDimension(int nDimension);

 protected:
  float m_fRadius;
  RawPoint m_oPointCenter;
  RawPoint m_oPointStart;
  RawPoint m_oPointEnd;
};
}  // namespace core
#endif  // GEOMETRY_CURVE_ARC_H