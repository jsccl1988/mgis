#ifndef GEOMETRY_GEOMETRY_H
#define GEOMETRY_GEOMETRY_H

#include "core/core.h"
#include "geometry/base/dot.h"
#include "geometry/base/matrix2d.h"
#include "geometry/base/rect.h"
#include "geometry/export.h"


namespace core {
using RawPoint = Dot2D<double>;
using Envelope = Rect2D<double>;
enum SpatialRelationship {
  SS_Unkown = 0,
  SS_Within = 1,
  SS_Touches = 1 << 1,
  SS_Crosses = 1 << 2,
  SS_Overlaps = 1 << 3,
  SS_Intersects = 1 << 4,
  SS_Equals = 1 << 5,
  SS_Contains = 1 << 6,
  SS_Disjoint = 1 << 7
};

enum GeometryType {
  GTUnknown = 0,
  GTPoint = 1,
  GTLineString = 2,
  GTSpline = 3,
  GTArc = 4,
  GTLinearRing = 5,
  GTPolygon = 6,
  GTFan = 7,
  GTMultiPoint = 8,
  GTMultiLineString = 9,
  GTMultiPolygon = 10,
  GTGeometryCollection = 11,
  GTGrid = 12,
  GTTin = 13,
  GTNone = 14
};

enum SplineType {
  CT_CIRCLE,
  CT_LAGSPLINE,
  CT_BZERSPLINE,
  CT_BSPLINE,
  CT_3SPLINE,
};

class GEOMETRY_EXPORT Geometry {
 public:
  Geometry();
  virtual ~Geometry();

  // standard Geometry
  virtual int GetDimension() const = 0;
  virtual int GetCoordinateDimension() const;
  virtual GeometryType GetGeometryType() const = 0;
  virtual const char *GetGeometryName() const = 0;

  virtual void SetCoordinateDimension(int nDimension);

  virtual bool IsEmpty() const = 0;
  virtual bool IsValid() const;
  virtual bool IsSimple() const;
  virtual bool IsRing() const;
  virtual void Empty() = 0;
  virtual Geometry *Clone() const = 0;
  virtual void GetEnvelope(Envelope *envelope) const = 0;

  // SpatialRelation
  virtual bool Intersects(const Geometry *) const;
  virtual bool Equals(const Geometry *) const = 0;
  virtual bool Disjoint(const Geometry *) const;
  virtual bool Touches(const Geometry *) const;
  virtual bool Crosses(const Geometry *) const;
  virtual bool Within(const Geometry *) const;
  virtual bool Contains(const Geometry *) const;
  virtual bool Overlaps(const Geometry *) const;

  virtual long Relationship(const Geometry *, float fMargin = 0.1) const;

  virtual Geometry *GetBoundary() const;
  virtual double Distance(const Geometry *) const;
  virtual Geometry *ConvexHull() const;
  virtual Geometry *Buffer(double distance, int quad_segs = 30) const;
  virtual Geometry *Intersection(const Geometry *) const;
  virtual Geometry *Union(const Geometry *) const;
  virtual Geometry *Difference(const Geometry *) const;
  virtual Geometry *SymmetricDifference(const Geometry *) const;

  // backward compatibility methods.
  bool Intersect(Geometry *) const;
  bool Equal(Geometry *) const;

 protected:
  int coord_dimension_;
};

class GEOMETRY_EXPORT GeometryCollection : public Geometry {
 public:
  GeometryCollection();
  virtual ~GeometryCollection();

  // Non standard (Geometry).
  virtual const char *GetGeometryName() const;
  virtual GeometryType GetGeometryType() const;
  virtual Geometry *Clone() const;
  virtual void Empty();

  virtual bool IsEmpty() const;

  virtual double GetArea() const;

  // Geometry methods
  virtual int GetDimension() const;
  virtual void GetEnvelope(Envelope *envelope) const;

  // GeometryCollection
  int GetNumGeometries() const;
  Geometry *GetGeometryRef(int);
  const Geometry *GetGeometryRef(int) const;

  // SpatialRelation
  virtual bool Equals(const Geometry *) const;

  // Non standard
  virtual void SetCoordinateDimension(int nDimension);
  virtual int AddGeometry(const Geometry *);
  virtual int AddGeometryDirectly(Geometry *);
  virtual int RemoveGeometry(int iIndex, int bDelete = TRUE);

  void CloseRings();

 private:
  int geometry_size_;
  Geometry **geometrys_;
};
}  // namespace core
#endif  // GEOMETRY_GEOMETRY_H