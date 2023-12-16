#include "geometry/curve/linestring.h"

#include "geometry/algorithms.h"

namespace core {
LineString::LineString() {
  point_size_ = 0;
  points_ = NULL;
}

LineString::LineString(LineString *pLS) {
  SetNumPoints(pLS->GetNumPoints());
  memcpy(points_, pLS->points_, sizeof(RawPoint) * GetNumPoints());
}

LineString::~LineString() {
  if (NULL != points_) {
    free(points_);
    point_size_ = 0;
  }
}

// Geometry interface
int LineString::GetDimension() const { return 2; }

GeometryType LineString::GetGeometryType() const { return GTLineString; }

const char *LineString::GetGeometryName() const { return "LINESTRING"; }

//////////////////////////////////////////////////////////////////////////
Geometry *LineString::Clone() const {
  LineString *poNewLineString;

  poNewLineString = new LineString();

  poNewLineString->SetPoints(point_size_, points_);
  poNewLineString->SetCoordinateDimension(GetCoordinateDimension());

  return poNewLineString;
}

void LineString::Empty() { SetNumPoints(0); }

void LineString::GetEnvelope(Envelope *envelope) const {
  if (NULL == envelope) return;
  double dfMinX, dfMinY, dfMaxX, dfMaxY;

  if (point_size_ == 0) return;

  dfMinX = dfMaxX = points_[0].x;
  dfMinY = dfMaxY = points_[0].y;

  for (int iPoint = 1; iPoint < point_size_; iPoint++) {
    if (dfMaxX < points_[iPoint].x) dfMaxX = points_[iPoint].x;
    if (dfMaxY < points_[iPoint].y) dfMaxY = points_[iPoint].y;
    if (dfMinX > points_[iPoint].x) dfMinX = points_[iPoint].x;
    if (dfMinY > points_[iPoint].y) dfMinY = points_[iPoint].y;
  }

  envelope->MinX = dfMinX;
  envelope->MaxX = dfMaxX;
  envelope->MinY = dfMinY;
  envelope->MaxY = dfMaxY;
}

bool LineString::IsEmpty() const { return (point_size_ == 0); }

// Curve methods
double LineString::GetLength() const {
  double dfLength = 0;
  int i;

  for (i = 0; i < point_size_ - 1; i++) {
    double dfDeltaX, dfDeltaY;

    dfDeltaX = points_[i + 1].x - points_[i].x;
    dfDeltaY = points_[i + 1].y - points_[i].y;
    dfLength += sqrt(dfDeltaX * dfDeltaX + dfDeltaY * dfDeltaY);
  }

  return dfLength;
}

void LineString::StartPoint(Point *poPoint) const { GetPoint(0, poPoint); }

void LineString::EndPoint(Point *poPoint) const {
  GetPoint(point_size_ - 1, poPoint);
}

void LineString::Value(double dfDistance, Point *poPoint) const {
  double dfLength = 0;
  int i;

  if (dfDistance < 0) {
    StartPoint(poPoint);
    return;
  }

  for (i = 0; i < point_size_ - 1; i++) {
    double dfDeltaX, dfDeltaY, dfSegLength;

    dfDeltaX = points_[i + 1].x - points_[i].x;
    dfDeltaY = points_[i + 1].y - points_[i].y;
    dfSegLength = sqrt(dfDeltaX * dfDeltaX + dfDeltaY * dfDeltaY);

    if (dfSegLength > 0) {
      if ((dfLength <= dfDistance) &&
          ((dfLength + dfSegLength) >= dfDistance)) {
        double dfRatio;

        dfRatio = (dfDistance - dfLength) / dfSegLength;

        poPoint->SetX(points_[i].x * (1 - dfRatio) +
                      points_[i + 1].x * dfRatio);
        poPoint->SetY(points_[i].y * (1 - dfRatio) +
                      points_[i + 1].y * dfRatio);

        return;
      }

      dfLength += dfSegLength;
    }
  }

  EndPoint(poPoint);
}

// LineString methods
void LineString::GetPoint(int i, Point *poPoint) const {
  assert(i >= 0);
  assert(i < point_size_);
  assert(poPoint != NULL);

  poPoint->SetX(points_[i].x);
  poPoint->SetY(points_[i].y);
}

void LineString::GetPoint(int i, RawPoint *rawPoint) const {
  assert(i >= 0);
  assert(i < point_size_);
  assert(poPoint != NULL);

  rawPoint->x = points_[i].x;
  rawPoint->y = points_[i].y;
}

// SpatialRelation
bool LineString::Equals(const Geometry *poOther) const {
  LineString *poOLine = (LineString *)poOther;

  if (poOLine == this) return true;

  if (poOther->GetGeometryType() != GetGeometryType()) return false;

  // we should eventually test the SRS.
  if (GetNumPoints() != poOLine->GetNumPoints()) return false;

  for (int iPoint = 0; iPoint < GetNumPoints(); iPoint++) {
    if (GetX(iPoint) != poOLine->GetX(iPoint) ||
        GetY(iPoint) != poOLine->GetY(iPoint))
      return false;
  }

  return true;
}

//
long LineString::Relationship(const Geometry *pOther, float fMargin) const {
  long lRet = SS_Unkown;
  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      Envelope env, oenv;
      GetEnvelope(&env);
      pOther->GetEnvelope(&oenv);

      if (!env.Intersects(oenv)) {
        lRet = SS_Disjoint;
      } else if (Distance(pOther) < fMargin) {
        lRet = SS_Overlaps;
      } else if (points_[point_size_ - 1].x == points_[0].x &&
                 points_[point_size_ - 1].y == points_[0].y) {
        if (point_size_ < 3) return SS_Unkown;

        Point *pPoint = (Point *)pOther;
        RawPoint rawPoint(pPoint->GetX(), pPoint->GetY());

        long flag = PointToPolygon_New1(rawPoint, points_, point_size_);
        if (flag == -1)
          lRet = SS_Contains;
        else if (flag == 0)
          lRet = SS_Overlaps;
        else
          lRet = SS_Disjoint;
      }
    } break;
    case GTLineString:
    case GTLinearRing:
    case GTSpline:
    case GTPolygon: {
      Envelope env, oenv;
      GetEnvelope(&env);
      pOther->GetEnvelope(&oenv);

      if (!env.Intersects(oenv)) {
        lRet = SS_Disjoint;
      } else if (points_[point_size_ - 1].x == points_[0].x &&
                 points_[point_size_ - 1].y == points_[0].y &&
                 env.Contains(oenv)) {
        lRet = SS_Contains;
      } else {
        lRet = SS_Intersects;
      }
    } break;
  }

  return lRet;
}

//
double LineString::Distance(const Geometry *pOther) const {
  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      int indexPre = 0, indexNext = 0;
      Point *pPoint = (Point *)pOther;
      RawPoint rawPoint(pPoint->GetX(), pPoint->GetY());
      return Distance_New(rawPoint, points_, point_size_, indexPre,
                          indexNext);
    } break;
    case GTLineString: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
    case GTPolygon: {
      return SMT_C_INVALID_DBF_VALUE;
    } break;
    default:
      return SMT_C_INVALID_DBF_VALUE;
  }

  return SMT_C_INVALID_DBF_VALUE;
}

// non standard.
void LineString::SetCoordinateDimension(int nDimension) {
  coord_dimension_ = nDimension;
}

void LineString::SetNumPoints(int nNewPointCount) {
  if (nNewPointCount == 0) {
    free(points_);
    point_size_ = 0;
    return;
  }

  if (nNewPointCount > point_size_) {
    points_ =
        (RawPoint *)realloc(points_, sizeof(RawPoint) * nNewPointCount);
    assert(points_ != NULL);
    memset(points_ + point_size_, 0,
           sizeof(RawPoint) * (nNewPointCount - point_size_));
  }

  point_size_ = nNewPointCount;
}

void LineString::SetPoint(int iPoint, Point *poPoint) {
  SetPoint(iPoint, poPoint->GetX(), poPoint->GetY());
}

void LineString::SetPoint(int iPoint, double xIn, double yIn) {
  if (iPoint >= point_size_) SetNumPoints(iPoint + 1);

  points_[iPoint].x = xIn;
  points_[iPoint].y = yIn;
}

void LineString::SetPoints(int nPointsIn, RawPoint *pPointsIn) {
  if (NULL == pPointsIn) return;

  SetNumPoints(nPointsIn);
  memcpy(points_, pPointsIn, sizeof(RawPoint) * nPointsIn);
}

void LineString::SetPoints(int nPointsIn, double *padfX, double *padfY) {
  SetNumPoints(nPointsIn);

  int i;

  for (i = 0; i < nPointsIn; i++) {
    points_[i].x = padfX[i];
    points_[i].y = padfY[i];
  }
}

void LineString::AddPoint(Point *poPoint) {
  SetPoint(point_size_, poPoint->GetX(), poPoint->GetY());
}

void LineString::AddPoint(double xIn, double yIn) {
  SetPoint(point_size_, xIn, yIn);
}

void LineString::GetPoints(RawPoint *poPoints) const {
  if (NULL == poPoints) return;
  memcpy(poPoints, points_, sizeof(RawPoint) * point_size_);
}

void LineString::AddSubLineString(const LineString *pSubLinestring,
                                  int nStartVertex, int nEndVertex) {
  if (nEndVertex == -1) nEndVertex = pSubLinestring->GetNumPoints() - 1;

  if (nStartVertex < 0 || nEndVertex < 0 ||
      nStartVertex >= pSubLinestring->GetNumPoints() ||
      nEndVertex >= pSubLinestring->GetNumPoints()) {
    return;
  }

  /* -------------------------------------------------------------------- */
  /*      Grow this linestring to hold the additional points.             */
  /* -------------------------------------------------------------------- */
  int nOldbfPoints = point_size_;
  int nPointsToAdd = abs(nEndVertex - nStartVertex) + 1;

  SetNumPoints(nPointsToAdd + nOldbfPoints);

  /* -------------------------------------------------------------------- */
  /*      Copy the x/y points - forward copies use memcpy.                */
  /* -------------------------------------------------------------------- */
  if (nEndVertex >= nStartVertex) {
    memcpy(points_ + nOldbfPoints, pSubLinestring->points_ + nStartVertex,
           sizeof(RawPoint) * nPointsToAdd);
  }

  /* -------------------------------------------------------------------- */
  /*      Copy the x/y points - reverse copies done double by double.     */
  /* -------------------------------------------------------------------- */
  else {
    int i;
    for (i = 0; i < nPointsToAdd; i++) {
      points_[i + nOldbfPoints].x =
          pSubLinestring->points_[nStartVertex - i].x;
      points_[i + nOldbfPoints].y =
          pSubLinestring->points_[nStartVertex - i].y;
    }
  }
}
}  // namespace core