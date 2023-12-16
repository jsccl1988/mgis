
#include "geometry/tin.h"

namespace core {
Tin::Tin()
    : triangles_(NULL),
      triangle_size_(0),
      points_(NULL),
      point_size_(0) {
  Empty();
}

Tin::~Tin() { Empty(); }

// Geometry
int Tin::GetDimension() const { return 0; }

const char *Tin::GetGeometryName() const { return "TIN"; }

GeometryType Tin::GetGeometryType() const { return GTTin; }

Geometry *Tin::Clone() const {
  Tin *poNewTin = new Tin();
  poNewTin->SetCoordinateDimension(coord_dimension_);

  // clone points
  int nPoints = point_size_;

  Point *pPoints = new Point[nPoints];
  for (int i = 0; i < nPoints; i++) {
    pPoints[i].SetX(points_[i].x);
    pPoints[i].SetY(points_[i].y);
  }

  poNewTin->AddPointCollection(pPoints, nPoints);

  // clone triangle
  poNewTin->AddTriangleCollection(triangles_, triangle_size_);

  SAFE_DELETE_A(pPoints);

  return poNewTin;
}

void Tin::Empty() {
  SAFE_DELETE_A(points_);
  SAFE_DELETE_A(triangles_);

  triangle_size_ = 0;

  coord_dimension_ = 0;
}

void Tin::GetEnvelope(Envelope *envelope) const {
  if (envelope == NULL || point_size_ < 1) return;

  envelope->MaxX = points_[0].x;
  envelope->MaxY = points_[0].y;
  envelope->MinX = points_[0].x;
  envelope->MinY = points_[0].y;

  for (int iPoint = 0; iPoint < point_size_; iPoint++)
    envelope->Merge(points_[iPoint].x, points_[iPoint].y);
}

bool Tin::IsEmpty() const { return coord_dimension_ == 0; }

// Non standard
void Tin::SetCoordinateDimension(int nDimension) {
  coord_dimension_ = nDimension;
}

// SpatialRelation
bool Tin::Equals(const Geometry *geometry) const {
  Tin *poTin = (Tin *)geometry;

  if (poTin == this) return true;

  if (poTin->GetGeometryType() != GetGeometryType()) return false;

  return false;
}

//////////////////////////////////////////////////////////////////////////
Triangle Tin::GetTriangle(int iIndex) const {
  if (iIndex < 0 || iIndex >= triangle_size_)
    return Triangle();
  else
    return triangles_[iIndex];
}

Point Tin::GetPoint(int iIndex) const {
  Point rawPoint;
  if (iIndex > -1 || iIndex < point_size_) {
    rawPoint.SetX(points_[iIndex].x);
    rawPoint.SetY(points_[iIndex].y);
  }

  return rawPoint;
}

//////////////////////////////////////////////////////////////////////////

int Tin::AddPoint(Point *poPoint) {
  points_ =
      (RawPoint *)realloc(points_, sizeof(RawPoint) * (point_size_ + 1));
  points_[point_size_].x = poPoint->GetX();
  points_[point_size_].y = poPoint->GetY();
  point_size_++;

  return ERR_NONE;
}

int Tin::AddPointCollection(Point *poPoint, int nPoints) {
  if (NULL == poPoint || nPoints < 1) return ERR_FAILURE;

  points_ = (RawPoint *)realloc(points_,
                                  sizeof(RawPoint) * (point_size_ + nPoints));

  for (int i = 0; i < nPoints; i++) {
    points_[point_size_ + i].x = poPoint[i].GetX();
    points_[point_size_ + i].y = poPoint[i].GetY();
  }

  point_size_ += nPoints;

  return ERR_NONE;
}

int Tin::AddPointCollection(dbfPoint *pPoints, int nPoints) {
  if (NULL == pPoints || nPoints < 1) return ERR_FAILURE;

  points_ = (RawPoint *)realloc(points_,
                                  sizeof(RawPoint) * (point_size_ + nPoints));

  for (int i = 0; i < nPoints; i++) {
    points_[point_size_ + i].x = pPoints[i].x;
    points_[point_size_ + i].y = pPoints[i].y;
  }

  point_size_ += nPoints;

  return ERR_NONE;
}

int Tin::AddTriangle(Triangle *poNewTri) {
  Triangle *poNewTri1 = new Triangle;

  memcpy(poNewTri1, poNewTri, sizeof(Triangle));

  triangles_ = (Triangle *)realloc(triangles_,
                                     sizeof(Triangle) * (triangle_size_ + 1));

  triangles_[triangle_size_] = *poNewTri;
  triangle_size_++;

  return ERR_NONE;
}

int Tin::AddTriangleCollection(Triangle *poTris, int nTris) {
  if (NULL == poTris || nTris < 1) return ERR_FAILURE;

  triangles_ = (Triangle *)realloc(
      triangles_, sizeof(Triangle) * (triangle_size_ + nTris));

  for (int i = 0; i < nTris; i++) {
    triangles_[triangle_size_ + i] = poTris[i];
  }

  triangle_size_ += nTris;

  return ERR_NONE;
}

int Tin::RemoveTriangle(int iIndex) {
  if (iIndex < -1 || iIndex >= triangle_size_) return ERR_FAILURE;

  // Special case.
  if (iIndex == -1) {
    while (triangle_size_ > 0) RemoveTriangle(triangle_size_ - 1);
    return ERR_NONE;
  }

  triangles_[iIndex].erase = true;

  return ERR_NONE;
}
}  // namespace core