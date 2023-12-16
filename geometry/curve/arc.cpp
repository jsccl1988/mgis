

#include "geometry/curve/arc.h"

#include "geometry/algorithms.h"

namespace core {
Arc::Arc() { m_fRadius = 0; }
Arc::~Arc() {}

// Geometry interface
int Arc::GetDimension() const { return 2; }

GeometryType Arc::GetGeometryType() const { return GTArc; }

const char *Arc::GetGeometryName() const { return "ARC"; }

//////////////////////////////////////////////////////////////////////////
Geometry *Arc::Clone() const {
  Arc *poArc;

  poArc = new Arc();
  poArc->SetCenterPoint(m_oPointCenter);
  poArc->SetStartPoint(m_oPointStart);
  poArc->SetEndPoint(m_oPointEnd);
  poArc->SetRadius(m_fRadius);

  return poArc;
}

void Arc::Empty() { m_fRadius = 0; }

void Arc::GetEnvelope(Envelope *envelope) const {
  if (NULL == envelope) {
    return;
  }

  envelope->Merge(m_oPointCenter.x - m_fRadius, m_oPointCenter.y - m_fRadius);
  envelope->Merge(m_oPointCenter.x + m_fRadius, m_oPointCenter.y + m_fRadius);
}

bool Arc::IsEmpty() const { return (m_fRadius == 0); }

// Curve methods
double Arc::GetLength() const {
  double dfLength = 0;
  double ax, ay, bx, by;
  double theta = 0.;
  ax = m_oPointStart.x - m_oPointCenter.x;
  ay = m_oPointStart.y - m_oPointCenter.y;

  bx = m_oPointEnd.x - m_oPointCenter.x;
  by = m_oPointEnd.y - m_oPointCenter.y;

  theta = acos((ax * bx + ay * by) / m_fRadius / m_fRadius);

  dfLength = m_fRadius * theta * dPI;

  return dfLength;
}

void Arc::StartPoint(Point *poPoint) const {
  poPoint->SetX(m_oPointStart.x);
  poPoint->SetY(m_oPointStart.y);
}

void Arc::EndPoint(Point *poPoint) const {
  poPoint->SetX(m_oPointEnd.x);
  poPoint->SetY(m_oPointEnd.y);
}

void Arc::Value(double dfDistance, Point *poPoint) const {
  double dfLength = 0;
  if (dfDistance < 0) {
    StartPoint(poPoint);
    return;
  }

  EndPoint(poPoint);
}

// SpatialRelation
bool Arc::Equals(const Geometry *poOther) const {
  Arc *poArc = (Arc *)poOther;

  if (poArc == this) return true;

  if (poArc->GetGeometryType() != GetGeometryType()) return false;

  if (IsEqual(poArc->GetRadius(), m_fRadius, dEPSILON)) return false;

  // we should eventually test the SRS.
  Point oCenter, oStart, oEnd;

  poArc->GetCenterPoint(&oCenter);
  poArc->StartPoint(&oStart);
  poArc->EndPoint(&oEnd);

  return (IsEqual(m_oPointCenter.x, oCenter.GetX(), dEPSILON) &&
          IsEqual(m_oPointCenter.x, oCenter.GetX(), dEPSILON) &&
          IsEqual(m_oPointStart.x, oStart.GetX(), dEPSILON) &&
          IsEqual(m_oPointStart.x, oStart.GetX(), dEPSILON) &&
          IsEqual(m_oPointEnd.x, oEnd.GetX(), dEPSILON) &&
          IsEqual(m_oPointEnd.x, oEnd.GetX(), dEPSILON));

  return true;
}

// non standard.
void Arc::SetCoordinateDimension(int nDimension) {
  coord_dimension_ = nDimension;
}
}  // namespace core