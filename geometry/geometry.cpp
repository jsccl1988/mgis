#include "geometry/geometry.h"

namespace core {
Geometry::Geometry() { coord_dimension_ = 2; }
Geometry::~Geometry() {}

void Geometry::SetCoordinateDimension(int nNewDimension) {
  coord_dimension_ = nNewDimension;
}

int Geometry::GetCoordinateDimension() const { return coord_dimension_; }

bool Geometry::IsValid() const { return false; }
bool Geometry::IsSimple() const { return false; }
bool Geometry::IsRing() const { return false; }

// SpatialRelation
bool Geometry::Intersects(const Geometry *geometry) const { return false; }
bool Geometry::Disjoint(const Geometry *geometry) const { return false; }
bool Geometry::Touches(const Geometry *geometry) const { return false; }
bool Geometry::Crosses(const Geometry *geometry) const { return false; }
bool Geometry::Within(const Geometry *geometry) const { return false; }
bool Geometry::Contains(const Geometry *geometry) const { return false; }
bool Geometry::Overlaps(const Geometry *geometry) const { return false; }

Geometry *Geometry::GetBoundary() const { return NULL; }
double Geometry::Distance(const Geometry *geometry) const { return -1.0; }
Geometry *Geometry::ConvexHull() const { return NULL; }
Geometry *Geometry::Buffer(double distance, int quad_segs) const {
  return NULL;
}
Geometry *Geometry::Intersection(const Geometry *geometry) const {
  return NULL;
}
Geometry *Geometry::Union(const Geometry *geometry) const { return NULL; }
Geometry *Geometry::Difference(const Geometry *geometry) const { return NULL; }

long Geometry::Relationship(const Geometry *, float fMargin) const {
  return -2;
}

Geometry *Geometry::SymmetricDifference(const Geometry *geometry) const {
  return NULL;
}

// backward compatibility methods.
bool Geometry::Intersect(Geometry *geometry) const { return NULL; }
bool Geometry::Equal(Geometry *geometry) const { return false; }

GeometryCollection::GeometryCollection() {
  geometry_size_ = 0;
  geometrys_ = NULL;
}

GeometryCollection::~GeometryCollection() { Empty(); }

// Non standard (Geometry).
const char *GeometryCollection::GetGeometryName() const {
  return "GEOMETRYCOLLECTION";
}

GeometryType GeometryCollection::GetGeometryType() const {
  return GTGeometryCollection;
}

Geometry *GeometryCollection::Clone() const {
  GeometryCollection *poNewGC;

  poNewGC = new GeometryCollection;
  for (int i = 0; i < geometry_size_; i++) {
    poNewGC->AddGeometry(geometrys_[i]);
  }

  return poNewGC;
}

void GeometryCollection::Empty() {
  if (NULL != geometrys_) {
    for (int i = 0; i < geometry_size_; i++) {
      SMT_SAFE_DELETE(geometrys_[i])
    }

    free(geometrys_);
  }

  geometry_size_ = 0;
}

bool GeometryCollection::IsEmpty() const {
  for (int iGeom = 0; iGeom < geometry_size_; iGeom++)
    if (geometrys_[iGeom]->IsEmpty() == false) return false;
  return true;
}

double GeometryCollection::GetArea() const {
  double dfArea = 0.0;
  for (int iGeom = 0; iGeom < geometry_size_; iGeom++) {
    Geometry *geom = geometrys_[iGeom];
    switch (geom->GetGeometryType()) {
      case GTPolygon:
        dfArea += ((Polygon *)geom)->GetArea();
        break;

      case GTMultiPolygon:
        dfArea += ((MultiPolygon *)geom)->GetArea();
        break;

      case GTLinearRing:
      case GTLineString:
        /* This test below is required to filter out wkbLineString geometries
         * not being of type of wkbLinearRing.
         */
        if (strcmp(((Geometry *)geom)->GetGeometryName(), "LINEARRING") == 0) {
          dfArea += ((LinearRing *)geom)->GetArea();
        }
        break;

      case GTGeometryCollection:
        dfArea += ((GeometryCollection *)geom)->GetArea();
        break;

      default:
        break;
    }
  }

  return dfArea;
}

// IGeometry methods
int GeometryCollection::GetDimension() const { return 2; }

void GeometryCollection::GetEnvelope(Envelope *envelope) const {
  Envelope oGeomEnv;

  if (geometry_size_ == 0) return;

  geometrys_[0]->GetEnvelope(envelope);

  for (int iGeom = 1; iGeom < geometry_size_; iGeom++) {
    geometrys_[iGeom]->GetEnvelope(&oGeomEnv);

    if (envelope->MinX > oGeomEnv.MinX) envelope->MinX = oGeomEnv.MinX;
    if (envelope->MinY > oGeomEnv.MinY) envelope->MinY = oGeomEnv.MinY;
    if (envelope->MaxX < oGeomEnv.MaxX) envelope->MaxX = oGeomEnv.MaxX;
    if (envelope->MaxY < oGeomEnv.MaxY) envelope->MaxY = oGeomEnv.MaxY;
  }
}

// IGeometryCollection
int GeometryCollection::GetNumGeometries() const { return geometry_size_; }

Geometry *GeometryCollection::GetGeometryRef(int i) {
  if (i < 0 || i >= geometry_size_)
    return NULL;
  else
    return geometrys_[i];
}

const Geometry *GeometryCollection::GetGeometryRef(int i) const {
  if (i < 0 || i >= geometry_size_)
    return NULL;
  else
    return geometrys_[i];
}

// ISpatialRelation
bool GeometryCollection::Equals(const Geometry *poOther) const {
  GeometryCollection *poOGC = (GeometryCollection *)poOther;

  if (poOGC == this) return true;

  if (poOther->GetGeometryType() != GetGeometryType()) return false;

  if (GetNumGeometries() != poOGC->GetNumGeometries()) return false;

  // we should eventually test the SRS.

  for (int iGeom = 0; iGeom < geometry_size_; iGeom++) {
    if (!GetGeometryRef(iGeom)->Equals(poOGC->GetGeometryRef(iGeom)))
      return false;
  }

  return true;
}

// Non standard
void GeometryCollection::SetCoordinateDimension(int nDimension) {
  for (int iGeom = 0; iGeom < geometry_size_; iGeom++)
    geometrys_[iGeom]->SetCoordinateDimension(nDimension);

  Geometry::SetCoordinateDimension(nDimension);
}

int GeometryCollection::AddGeometry(const Geometry *poNewGeom) {
  Geometry *poClone = poNewGeom->Clone();

  int eErr;

  eErr = AddGeometryDirectly(poClone);
  if (eErr != SMT_ERR_NONE) delete poClone;

  return eErr;
}

int GeometryCollection::AddGeometryDirectly(Geometry *poNewGeom) {
  geometrys_ =
      (Geometry **)realloc(geometrys_, sizeof(void *) * (geometry_size_ + 1));

  geometrys_[geometry_size_] = poNewGeom;

  geometry_size_++;

  return SMT_ERR_NONE;
}

int GeometryCollection::RemoveGeometry(int iGeom, int bDelete) {
  if (iGeom < -1 || iGeom >= geometry_size_) return SMT_ERR_FAILURE;

  // Special case.
  if (iGeom == -1) {
    while (geometry_size_ > 0) RemoveGeometry(geometry_size_ - 1, bDelete);
    return SMT_ERR_NONE;
  }

  if (bDelete) SMT_SAFE_DELETE(geometrys_[iGeom]);

  memmove(geometrys_ + iGeom, geometrys_ + iGeom + 1,
          sizeof(void *) * (geometry_size_ - iGeom - 1));

  geometry_size_--;

  return SMT_ERR_NONE;
}

void GeometryCollection::CloseRings() {
  for (int iGeom = 0; iGeom < geometry_size_; iGeom++) {
    if (geometrys_[iGeom]->GetGeometryType() == GTPolygon)
      ((Polygon *)geometrys_[iGeom])->CloseRings();
  }
}
}  // namespace core