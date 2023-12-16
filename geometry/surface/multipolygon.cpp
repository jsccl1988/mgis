#include "geometry/surface/multipolygon.h"

#include "geometry/algorithms.h"

namespace core {
MultiPolygon::MultiPolygon() {}

MultiPolygon::~MultiPolygon() {}

// Non standard (Geometry).
const char *MultiPolygon::GetGeometryName() const { return "MULTIPOLYGON"; }

GeometryType MultiPolygon::GetGeometryType() const { return GTMultiPolygon; }

Geometry *MultiPolygon::Clone() const {
  MultiPolygon *poNewGC;

  poNewGC = new MultiPolygon;

  for (int i = 0; i < GetNumGeometries(); i++) {
    poNewGC->AddGeometry(GetGeometryRef(i));
  }

  return poNewGC;
}

// Non standard
int MultiPolygon::AddGeometryDirectly(Geometry *poNewGeom) {
  if (poNewGeom->GetGeometryType() != GTPolygon)
    return SMT_ERR_UNSUPPORTED_GEOTYPE;

  return GeometryCollection::AddGeometryDirectly(poNewGeom);
}

double MultiPolygon::GetArea() const {
  double dfArea = 0.0;
  int iPoly;

  for (iPoly = 0; iPoly < GetNumGeometries(); iPoly++) {
    Polygon *poPoly = (Polygon *)GetGeometryRef(iPoly);

    dfArea += poPoly->GetArea();
  }

  return dfArea;
}
}  // namespace core