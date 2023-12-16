#ifndef GEOMETRY_CURVE_LINESTRING_H
#define GEOMETRY_CURVE_LINESTRING_H

#include "geometry/geometry.h"

namespace core {
class GEOMETRY_EXPORT LineString : public Curve {
 public:
  LineString();
  LineString(LineString *pLS);
  virtual ~LineString();

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
  int GetNumPoints() const { return point_size_; }
  void GetPoint(int, Point *) const;
  void GetPoint(int, RawPoint *) const;

  double GetX(int i) const { return points_[i].x; }
  double GetY(int i) const { return points_[i].y; }

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;

  //
  virtual double Distance(const Geometry *) const;

  // non standard.
  virtual void SetCoordinateDimension(int nDimension);
  void SetNumPoints(int);
  void SetPoint(int, Point *);
  void SetPoint(int, double, double);
  void SetPoints(int, RawPoint *);
  void SetPoints(int, double *padfX, double *padfY);
  void AddPoint(Point *);
  void AddPoint(double, double);

  void GetPoints(RawPoint *) const;

  void AddSubLineString(const LineString *, int nStartVertex = 0,
                        int nEndVertex = -1);

 protected:
  int point_size_;
  RawPoint *points_;
};
}  // namespace core
#endif  // GEOMETRY_CURVE_LINESTRING_H