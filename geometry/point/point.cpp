
#include "geometry/point/point.h"

#include "geometry/algorithms.h"

#include "geometry/curve/linearring.h"
#include "geometry/curve/linestring.h"
#include "geometry/curve/spline.h"
#include "geometry/curve/arc.h"
#include "geometry/surface/fan.h"
#include "geometry/surface/multipolygon.h"

namespace core {
Point::Point() { Empty(); }

Point::Point(double xIn, double yIn) {
  x = xIn;
  y = yIn;
  coord_dimension_ = 2;
}

Point::~Point() {}

// Geometry
int Point::GetDimension() const { return 0; }

const char *Point::GetGeometryName() const { return "POINT"; }

GeometryType Point::GetGeometryType() const { return GTPoint; }

Geometry *Point::Clone() const {
  Point *poNewPoint = new Point(x, y);
  poNewPoint->SetCoordinateDimension(coord_dimension_);
  return poNewPoint;
}

void Point::Empty() {
  x = y = 0.0;
  coord_dimension_ = 0;
}

void Point::GetEnvelope(Envelope *envelope) const {
  envelope->MinX = GetX() - 1;
  envelope->MaxX = GetX() + 1;
  envelope->MinY = GetY() - 1;
  envelope->MaxY = GetY() + 1;
}

bool Point::IsEmpty() const { return coord_dimension_ == 0; }

// Non standard
void Point::SetCoordinateDimension(int nDimension) {
  coord_dimension_ = nDimension;
}

// SpatialRelation
bool Point::Equals(const Geometry *geometry) const {
  Point *poOPoint = (Point *)geometry;

  if (poOPoint == this) return true;

  if (geometry->GetGeometryType() != GetGeometryType()) return false;

  // we should eventually test the SRS.

  if (poOPoint->GetX() != GetX() || poOPoint->GetY() != GetY())
    return false;
  else
    return true;
}

long Point::Relationship(const Geometry *other, float fMargin) const {
  long lRet;
  switch (other->GetGeometryType()) {
    case GTPoint: {
      if (Distance(other) < fMargin) {
        lRet = SS_Overlaps;
      } else
        lRet = SS_Disjoint;
    } break;
    case GTLineString: {
      lRet = other->Relationship(this, fMargin);
    } break;
    case GTPolygon:
    case GTLinearRing:
    case GTSpline: {
      lRet = other->Relationship(this, fMargin);
      switch (lRet) {
        case SS_Contains:
          lRet = SS_Within;
          break;
        case SS_Disjoint:
          lRet = SS_Disjoint;
          break;
      }
    } break;
    default: {
      lRet = SS_Unkown;
    } break;
  }

  return lRet;
}

double Point::Distance(const Geometry *other) const {
  switch (other->GetGeometryType()) {
    case GTPoint: {
      Point *pPoint = (Point *)other;
      return Distance(GetX(), GetY(), pPoint->GetX(), pPoint->GetY());
    } break;
    case GTLineString: {
      LineString *pLineString = (LineString *)other;
      return pLineString->Distance(this);
    } break;
    case GTPolygon: {
      Polygon *pPoly = (Polygon *)other;
      return pPoly->Distance(this);
    } break;
    default: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
  }

  return SMT_C_INVALID_DBF_VALUE;
}
}  // namespace core