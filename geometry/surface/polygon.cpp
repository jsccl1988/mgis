#include "geometry/surface/polygon.h"

#include "geometry/algorithms.h"

namespace core {
Polygon::Polygon() {
  m_nRingCount = 0;
  m_pRings = NULL;
}

Polygon::~Polygon() { Empty(); }

// Non standard (Geometry).
const char *Polygon::GetGeometryName() const { return "POLYGON"; }

GeometryType Polygon::GetGeometryType() const { return GTPolygon; }

Geometry *Polygon::Clone() const {
  Polygon *poNewPolygon;

  poNewPolygon = new Polygon;

  for (int i = 0; i < m_nRingCount; i++) {
    poNewPolygon->AddRing(m_pRings[i]);
  }

  return poNewPolygon;
}

void Polygon::Empty() {
  if (NULL != m_pRings) {
    for (int i = 0; i < m_nRingCount; i++) {
      SMT_SAFE_DELETE(m_pRings[i])
    }

    free(m_pRings);
  }

  m_nRingCount = 0;
}

bool Polygon::IsEmpty() const {
  for (int iRing = 0; iRing < m_nRingCount; iRing++)
    if (m_pRings[iRing]->IsEmpty() == false) return false;
  return true;
}

// Surface Interface
double Polygon::GetArea() const {
  double dfArea = 0.0;

  if (GetExteriorRing() != NULL) {
    int iRing;

    dfArea = GetExteriorRing()->GetArea();

    for (iRing = 0; iRing < GetNumInteriorRings(); iRing++)
      dfArea -= GetInteriorRing(iRing)->GetArea();
  }

  return dfArea;
}

int Polygon::Centroid(Point *poPoint) const {
  if (poPoint == NULL) return SMT_ERR_FAILURE;

  return SMT_ERR_UNSUPPORTED;
}

int Polygon::PointOnSurface(Point *poPoint) const {
  if (poPoint == NULL) return SMT_ERR_FAILURE;

  return SMT_ERR_UNSUPPORTED;
}

// Geometry
int Polygon::GetDimension() const { return 2; }

void Polygon::GetEnvelope(Envelope *envelope) const {
  Envelope oRingEnv;

  if (m_nRingCount == 0) return;

  m_pRings[0]->GetEnvelope(envelope);

  for (int iRing = 1; iRing < m_nRingCount; iRing++) {
    m_pRings[iRing]->GetEnvelope(&oRingEnv);

    if (envelope->MinX > oRingEnv.MinX) envelope->MinX = oRingEnv.MinX;
    if (envelope->MinY > oRingEnv.MinY) envelope->MinY = oRingEnv.MinY;
    if (envelope->MaxX < oRingEnv.MaxX) envelope->MaxX = oRingEnv.MaxX;
    if (envelope->MaxY < oRingEnv.MaxY) envelope->MaxY = oRingEnv.MaxY;
  }
}

// SpatialRelation
bool Polygon::Equals(const Geometry *poOther) const {
  Polygon *poOPoly = (Polygon *)poOther;

  if (poOPoly == this) return TRUE;

  if (poOther->GetGeometryType() != GetGeometryType()) return FALSE;

  if (GetNumInteriorRings() != poOPoly->GetNumInteriorRings()) return FALSE;

  if (GetExteriorRing() == NULL && poOPoly->GetExteriorRing() == NULL)
    /* ok */;
  else if (GetExteriorRing() == NULL || poOPoly->GetExteriorRing() == NULL)
    return FALSE;
  else if (!GetExteriorRing()->Equals(poOPoly->GetExteriorRing()))
    return FALSE;

  // we should eventually test the SRS.

  for (int iRing = 0; iRing < GetNumInteriorRings(); iRing++) {
    if (!GetInteriorRing(iRing)->Equals(poOPoly->GetInteriorRing(iRing)))
      return FALSE;
  }

  return TRUE;
}

//
long Polygon::Relationship(const Geometry *pOther, float fMargin) const {
  long lRet = SS_Unkown;
  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      Envelope env, oenv;
      GetEnvelope(&env);
      pOther->GetEnvelope(&oenv);
      if (!env.Intersects(oenv)) {
        lRet = SS_Disjoint;
      } else {
        const LinearRing *pOutRing = GetExteriorRing();
        lRet = pOutRing->Relationship(pOther, fMargin);

        if (SS_Contains == lRet) {
          int nInterRNum = GetNumInteriorRings();
          for (int i = 0; i < nInterRNum; i++) {
            const LinearRing *pInterRing = GetInteriorRing(i);
            if (SS_Disjoint != pInterRing->Relationship(pOther, fMargin)) {
              lRet = SS_Disjoint;
            }
          }
        }
      }
    } break;
    case GTLineString: {
      lRet = SS_Unkown;
    } break;
    case GTPolygon: {
      lRet = SS_Unkown;
    } break;
  }

  return lRet;
}

//
double Polygon::Distance(const Geometry *pOther) const {
  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
    case GTLineString: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
    case GTPolygon: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
    default:
      return SMT_C_INVALID_DBF_VALUE;
      break;
  }

  return SMT_C_INVALID_DBF_VALUE;
}

// Non standard
void Polygon::SetCoordinateDimension(int nDimension) {
  for (int iRing = 0; iRing < m_nRingCount; iRing++)
    m_pRings[iRing]->SetCoordinateDimension(nDimension);

  Geometry::SetCoordinateDimension(nDimension);
}

void Polygon::AddRing(LinearRing *poNewRing) {
  m_pRings =
      (LinearRing **)realloc(m_pRings, sizeof(void *) * (m_nRingCount + 1));
  m_pRings[m_nRingCount] = new LinearRing(poNewRing);
  m_nRingCount++;
}

void Polygon::AddRingDirectly(LinearRing *poNewRing) {
  m_pRings =
      (LinearRing **)realloc(m_pRings, sizeof(void *) * (m_nRingCount + 1));
  m_pRings[m_nRingCount] = poNewRing;
  m_nRingCount++;
}

LinearRing *Polygon::GetExteriorRing() {
  if (m_nRingCount > 0)
    return m_pRings[0];
  else
    return NULL;
}

const LinearRing *Polygon::GetExteriorRing() const {
  if (m_nRingCount > 0)
    return m_pRings[0];
  else
    return NULL;
}

int Polygon::GetNumInteriorRings() const {
  if (m_nRingCount > 0)
    return m_nRingCount - 1;
  else
    return 0;
}

LinearRing *Polygon::GetInteriorRing(int iRing) {
  if (iRing < 0 || iRing >= m_nRingCount - 1)
    return NULL;
  else
    return m_pRings[iRing + 1];
}

const LinearRing *Polygon::GetInteriorRing(int iRing) const {
  if (iRing < 0 || iRing >= m_nRingCount - 1)
    return NULL;
  else
    return m_pRings[iRing + 1];
}

bool Polygon::IsPointOnSurface(const Point *pt) const {
  if (NULL == pt) return 0;

  for (int iRing = 0; iRing < m_nRingCount; iRing++) {
    if (m_pRings[iRing]->IsPointInRing(pt)) {
      return 1;
    }
  }

  return 0;
}

void Polygon::CloseRings() {
  for (int iRing = 0; iRing < m_nRingCount; iRing++)
    m_pRings[iRing]->CloseRings();
}

}  // namespace core