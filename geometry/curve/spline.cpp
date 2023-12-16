#include "geometry/curve/spline.h"

#include "geometry/algorithms.h"

namespace core {
Spline::Spline()
    : analytic_points_(NULL),
      analytic_type_(CT_BSPLINE),
      analytic_point_size_(0) {}

Spline::Spline(Spline *poLr)
    : analytic_points_(NULL),
      analytic_type_(CT_BSPLINE),
      analytic_point_size_(0) {
  SetNumPoints(poLr->GetNumPoints());
  memcpy(points_, poLr->points_, sizeof(RawPoint) * GetNumPoints());
}

Spline::~Spline() {
  SMT_SAFE_DELETE_A(analytic_points_);
  analytic_point_size_ = 0;
}

// Non standard.
const char *Spline::GetGeometryName() const { return "SPLINE"; }

GeometryType Spline::GetGeometryType() const { return GTSpline; }

Geometry *Spline::Clone() const {
  Spline *poNewSpline = new Spline();
  poNewSpline->SetPoints(point_size_, points_);
  poNewSpline->SetAnalyticType(analytic_type_);
  poNewSpline->CalcAnalyticPoints();

  return poNewSpline;
}

void Spline::CalcAnalyticPoints(void) {
  //
  SMT_SAFE_DELETE_A(analytic_points_);
  analytic_point_size_ = 0;

  //
  //...
  switch (analytic_type_) {
    case CT_LAGSPLINE:
      Interpolate_Lugrange(analytic_points_, analytic_point_size_, points_,
                           point_size_);
      break;
    case CT_BZERSPLINE:
      Interpolate_Geometry(analytic_points_, analytic_point_size_, points_,
                           point_size_);
      break;
    case CT_BSPLINE:
      Interpolate_BSpline(analytic_points_, analytic_point_size_, points_,
                          point_size_);
      break;
    case CT_3SPLINE:
      Interpolate_3Spline(analytic_points_, analytic_point_size_, points_,
                          point_size_);
      break;
    default:
      break;
  }
}

void Spline::GetAnalyticPoints(RawPoint *poPoints) const {
  if (NULL == poPoints) return;

  memcpy(poPoints, analytic_points_, sizeof(RawPoint) * analytic_point_size_);
}

void Spline::GetEnvelope(Envelope *envelope) const {
  if (NULL == envelope) return;
  double dfMinX, dfMinY, dfMaxX, dfMaxY;

  if (point_size_ == 0) return;

  LineString::GetEnvelope(envelope);

  if (NULL != analytic_points_) {
    dfMinX = dfMaxX = analytic_points_[0].x;
    dfMinY = dfMaxY = analytic_points_[0].y;

    for (int iPoint = 1; iPoint < analytic_point_size_; iPoint++) {
      if (dfMaxX < analytic_points_[iPoint].x)
        dfMaxX = analytic_points_[iPoint].x;
      if (dfMaxY < analytic_points_[iPoint].y)
        dfMaxY = analytic_points_[iPoint].y;
      if (dfMinX > analytic_points_[iPoint].x)
        dfMinX = analytic_points_[iPoint].x;
      if (dfMinY > analytic_points_[iPoint].y)
        dfMinY = analytic_points_[iPoint].y;
    }

    envelope->Merge(dfMinX, dfMinY);
    envelope->Merge(dfMaxX, dfMaxY);
  }
}

// Curve methods
double Spline::GetLength() const {
  double dfLength = 0;
  int i;

  if (NULL != analytic_points_) {
    for (i = 0; i < analytic_point_size_ - 1; i++) {
      double dfDeltaX, dfDeltaY;

      dfDeltaX = analytic_points_[i + 1].x - analytic_points_[i].x;
      dfDeltaY = analytic_points_[i + 1].y - analytic_points_[i].y;
      dfLength += sqrt(dfDeltaX * dfDeltaX + dfDeltaY * dfDeltaY);
    }

    return dfLength;
  } else {
    return LineString::GetLength();
  }
}

void Spline::Value(double dfDistance, Point *poPoint) const {
  double dfLength = 0;
  int i;

  if (dfDistance < 0) {
    StartPoint(poPoint);
    return;
  }

  for (i = 0; i < analytic_point_size_ - 1; i++) {
    double dfDeltaX, dfDeltaY, dfSegLength;

    dfDeltaX = analytic_points_[i + 1].x - analytic_points_[i].x;
    dfDeltaY = analytic_points_[i + 1].y - analytic_points_[i].y;
    dfSegLength = sqrt(dfDeltaX * dfDeltaX + dfDeltaY * dfDeltaY);

    if (dfSegLength > 0) {
      if ((dfLength <= dfDistance) &&
          ((dfLength + dfSegLength) >= dfDistance)) {
        double dfRatio;

        dfRatio = (dfDistance - dfLength) / dfSegLength;

        poPoint->SetX(analytic_points_[i].x * (1 - dfRatio) +
                      analytic_points_[i + 1].x * dfRatio);
        poPoint->SetY(analytic_points_[i].y * (1 - dfRatio) +
                      analytic_points_[i + 1].y * dfRatio);

        return;
      }

      dfLength += dfSegLength;
    }
  }

  EndPoint(poPoint);
}

//
long Spline::Relationship(const Geometry *pOther, float fMargin) const {
  if (NULL == analytic_points_)
    return LineString::Relationship(pOther, fMargin);

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
      } else if (analytic_points_[analytic_point_size_ - 1].x == points_[0].x &&
                 points_[analytic_point_size_ - 1].y == points_[0].y) {
        if (point_size_ < 3) return SS_Unkown;

        Point *pPoint = (Point *)pOther;
        RawPoint rawPoint(pPoint->GetX(), pPoint->GetY());

        long flag = PointToPolygon_New1(rawPoint, analytic_points_,
                                        analytic_point_size_);
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
    case GTPolygon: {
      Envelope env, oenv;
      GetEnvelope(&env);
      pOther->GetEnvelope(&oenv);

      if (!env.Intersects(oenv)) {
        lRet = SS_Disjoint;
      } else if (analytic_points_[analytic_point_size_ - 1].x ==
                     analytic_points_[0].x &&
                 analytic_points_[analytic_point_size_ - 1].y ==
                     analytic_points_[0].y &&
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
double Spline::Distance(const Geometry *pOther) const {
  if (NULL == analytic_points_) return LineString::Distance(pOther);

  switch (pOther->GetGeometryType()) {
    case GTPoint: {
      int indexPre = 0, indexNext = 0;
      Point *pPoint = (Point *)pOther;
      RawPoint rawPoint(pPoint->GetX(), pPoint->GetY());
      return Distance_New(rawPoint, analytic_points_, analytic_point_size_,
                          indexPre, indexNext);
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
}  // namespace core