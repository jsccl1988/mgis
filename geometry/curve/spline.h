#ifndef GEOMETRY_CURVE_SPLINE_H
#define GEOMETRY_CURVE_SPLINE_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT Spline : public LineString {
 public:
  Spline();
  Spline(Spline *);
  virtual ~Spline();

  // Non standard.
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;
  virtual Geometry *Clone() const;

  virtual void GetEnvelope(Envelope *envelope) const;
  virtual void Value(double, Point *) const;
  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;

  virtual double Distance(const Geometry *) const;

  inline long GetAnalyticType() const { return analytic_type_; }
  inline void SetAnalyticType(long type) { analytic_type_ = type; }
  void CalcAnalyticPoints(void);

  double GetLength() const;

  void GetAnalyticPoints(RawPoint *) const;
  double GetAnalyticX(int i) const { return analytic_points_[i].x; }
  double GetAnalyticY(int i) const { return analytic_points_[i].y; }
  inline long GetAnalyticPointCount(void) const { return analytic_point_size_; }

 protected:
  int analytic_type_;
  int analytic_point_size_;
  RawPoint *analytic_points_;
};

}  // namespace core
#endif  // GEOMETRY_CURVE_SPLINE_H