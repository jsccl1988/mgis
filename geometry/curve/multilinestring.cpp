#include "geometry/curve/multilinestring.h"

#include "geometry/algorithms.h"

namespace core {
MultiLineString::MultiLineString() {}
MultiLineString::~MultiLineString() {}

// Non standard (Geometry).
const char *MultiLineString::GetGeometryName() const {
  return "MULTILINESTRING";
}

GeometryType MultiLineString::GetGeometryType() const {
  return GTMultiLineString;
}

Geometry *MultiLineString::Clone() const {
  MultiLineString *poNewGC;

  poNewGC = new MultiLineString;

  for (int i = 0; i < GetNumGeometries(); i++) {
    poNewGC->AddGeometry(GetGeometryRef(i));
  }

  return poNewGC;
}

// Non standard
int MultiLineString::AddGeometryDirectly(Geometry *poNewGeom) {
  if (poNewGeom->GetGeometryType() != GTLineString)
    return SMT_ERR_UNSUPPORTED_GEOTYPE;

  return GeometryCollection::AddGeometryDirectly(poNewGeom);
}

}  // namespace core