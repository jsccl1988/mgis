#include "geometry/point/multipoint.h"

#include "geometry/algorithms.h"
#include "geometry/curve/arc.h"
#include "geometry/curve/linearring.h"
#include "geometry/curve/linestring.h"
#include "geometry/curve/spline.h"
#include "geometry/point/point.h"
#include "geometry/surface/fan.h"
#include "geometry/surface/multipolygon.h"

namespace core {
MultiPoint::MultiPoint() {}
MultiPoint::~MultiPoint() {}

// Non standard (Geometry).
const char *MultiPoint::GetGeometryName() const { return "MULTIPOINT"; }
GeometryType MultiPoint::GetGeometryType() const { return GTMultiPoint; }

Geometry *MultiPoint::Clone() const {
  MultiPoint *poNewGC;

  poNewGC = new MultiPoint;

  for (int i = 0; i < GetNumGeometries(); i++) {
    poNewGC->AddGeometry(GetGeometryRef(i));
  }

  return poNewGC;
}

void MultiPoint::GetEnvelope(Envelope *envelope) const {
  Point *pPoint = NULL;

  for (int i = 0; i < GetNumGeometries(); i++) {
    pPoint = (Point *)GetGeometryRef(i);
    envelope->Merge(pPoint->GetX(), pPoint->GetY());
  }
}

// Non standard
int MultiPoint::AddGeometryDirectly(Geometry *poNewGeom) {
  if (poNewGeom->GetGeometryType() != GTPoint)
    return SMT_ERR_UNSUPPORTED_GEOTYPE;

  return GeometryCollection::AddGeometryDirectly(poNewGeom);
}

long MultiPoint::Relationship(const Geometry *pOther, float fMargin) const {
  Envelope env;
  this->GetEnvelope(&env);

  long lRet;
  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      Point *pPoint = (Point *)pOther;
      if (env.Contains(pPoint->GetX(), pPoint->GetY())) {
        lRet = SS_Overlaps;
      } else
        lRet = SS_Disjoint;
    } break;
    case GTLineString:
    case GTPolygon:
    case GTLinearRing:
    case GTSpline: {
      Envelope envOther;
      pOther->GetEnvelope(&envOther);

      if (env.Contains(envOther))
        lRet = SS_Contains;
      else if (env.Intersects(envOther))
        lRet = SS_Intersects;
      else
        lRet = SS_Disjoint;
    } break;
    default: {
      lRet = SS_Unkown;
    } break;
  }

  return lRet;
}
}  // namespace core