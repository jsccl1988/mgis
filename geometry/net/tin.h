#ifndef GEOMETRY_NET_TIN_H
#define GEOMETRY_NET_TIN_H

#include "geometry/geometry.h"

namespace core {
struct Triangle {
  long a, b, c;
  bool erase;

  Triangle() : erase(false) { a = b = c = -1; }
};

class GEOMETRY_EXPORT Tin : public Geometry {
 public:
  Tin(void);
  virtual ~Tin(void);

 public:
  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;

  virtual Geometry *Clone() const;
  virtual void Empty();
  virtual bool IsEmpty() const;

  // Geometry
  virtual int GetDimension() const;
  virtual void GetEnvelope(Envelope *envelope) const;
  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  // Non standard
  virtual void SetCoordinateDimension(int nDimension);

  Point GetPoint(int) const;
  int GetPointCount() const { return point_size_; }

  Triangle GetTriangle(int) const;
  int GetTriangleCount() const { return triangle_size_; }

  int AddPoint(Point *);
  int AddPointCollection(Point *, int);
  int AddPointCollection(dbfPoint *, int);
  int AddTriangle(Triangle *);
  int AddTriangleCollection(Triangle *, int);

  int RemoveTriangle(int iIndex);

 protected:
  int point_size_;
  RawPoint *points_;

  int triangle_size_;
  Triangle *triangles_;
};
}  // namespace core
#endif  // GEOMETRY_NET_TIN_H