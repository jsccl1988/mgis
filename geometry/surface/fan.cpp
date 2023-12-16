
#include "geometry/surface/fan.h"

#include "geometry/algorithms.h"

namespace core {
Fan::Fan() { m_pArc = NULL; }
Fan::~Fan() { Empty(); }

// Non standard (Geometry).
const char *Fan::GetGeometryName() const { return "FAN"; }
GeometryType Fan::GetGeometryType() const { return GTFan; }

Geometry *Fan::Clone() const {
  Fan *poNewFan;
  poNewFan = new Fan();
  poNewFan->SetArc(m_pArc);
  return poNewFan;
}

void Fan::Empty() { SAFE_DELETE(m_pArc); }

bool Fan::IsEmpty() const {
  if (!m_pArc) return false;

  return m_pArc->IsEmpty();
}

// Surface Interface
double Fan::GetArea() const {
  double dfArea = 0.0;
  double ax, ay, bx, by;
  double theta = 0.;
  double fRadius = 0.;

  Point oCenter, oStart, oEnd;

  m_pArc->GetCenterPoint(&oCenter);
  m_pArc->StartPoint(&oStart);
  m_pArc->EndPoint(&oEnd);
  fRadius = m_pArc->GetRadius();

  ax = oStart.GetX() - oCenter.GetX();
  ay = oStart.GetY() - oCenter.GetY();

  bx = oEnd.GetX() - oCenter.GetX();
  by = oEnd.GetY() - oCenter.GetY();

  theta = acos((ax * bx + ay * by) / fRadius / fRadius);

  dfArea = fRadius * fRadius * theta / 2.;

  return dfArea;
}

int Fan::Centroid(Point *poPoint) const {
  if (poPoint == NULL) return ERR_FAILURE;

  return ERR_UNSUPPORTED;
}

int Fan::PointOnSurface(Point *poPoint) const {
  if (poPoint == NULL) return ERR_FAILURE;

  return ERR_UNSUPPORTED;
}

// Geometry
int Fan::GetDimension() const { return 2; }

void Fan::GetEnvelope(Envelope *envelope) const {
  if (m_pArc) m_pArc->GetEnvelope(envelope);
}

// SpatialRelation
bool Fan::Equals(const Geometry *poOther) const {
  Fan *poFan = (Fan *)poOther;

  if (poFan == this) return TRUE;

  if (poOther->GetGeometryType() != GetGeometryType()) return FALSE;

  return poFan->GetArc()->Equals(m_pArc);
}

// Non standard
void Fan::SetCoordinateDimension(int nDimension) {
  m_pArc->SetCoordinateDimension(nDimension);
  Geometry::SetCoordinateDimension(nDimension);
}

void Fan::SetArc(Arc *poArc) {
  Empty();

  m_pArc = new Arc();
  Point oCenter, oStart, oEnd;
  poArc->GetCenterPoint(&oCenter);
  poArc->StartPoint(&oStart);
  poArc->EndPoint(&oEnd);

  RawPoint oCt(oCenter.GetX(), oCenter.GetY()),
      oSt(oStart.GetX(), oStart.GetY()), oEd(oEnd.GetX(), oEnd.GetY());

  m_pArc->SetCenterPoint(oCt);
  m_pArc->SetStartPoint(oSt);
  m_pArc->SetEndPoint(oEd);
  m_pArc->SetRadius(poArc->GetRadius());
}

void Fan::SetArcDirectly(Arc *poArc) {
  Empty();
  m_pArc = poArc;
}

}  // namespace core