#include "geometry/curve//linearring.h"

#include "geometry/algorithms.h"

namespace core {
LinearRing::LinearRing() {}

LinearRing::LinearRing(LinearRing *poLr) {
  SetNumPoints(poLr->GetNumPoints());
  memcpy(points_, poLr->points_, sizeof(RawPoint) * GetNumPoints());
}

LinearRing::~LinearRing() {}

// Non standard.
const char *LinearRing::GetGeometryName() const { return "LINEARRING"; }

GeometryType LinearRing::GetGeometryType() const { return GTLinearRing; }

Geometry *LinearRing::Clone() const {
  LinearRing *poNewLinearRing;

  poNewLinearRing = new LinearRing();
  poNewLinearRing->SetPoints(point_size_, points_);

  return poNewLinearRing;
}

bool LinearRing::IsClockwise() const {
  int i, v, next;
  double dx0, dy0, dx1, dy1, crossproduct;

  if (point_size_ < 2) return true;

  /* Find the lowest rightmost vertex */
  v = 0;
  for (i = 1; i < point_size_ - 1; i++) {
    /* => v < end */
    if (points_[i].y < points_[v].y ||
        (points_[i].y == points_[v].y && points_[i].x > points_[v].x)) {
      v = i;
    }
  }

  /* Vertices may be duplicate, we have to go to nearest different in each
   * direction */
  /* preceding */
  next = v - 1;
  while (1) {
    if (next < 0) {
      next = point_size_ - 1 - 1;
    }

    if (!IsEqual(points_[next].x, points_[v].x, dEPSILON) ||
        !IsEqual(points_[next].y, points_[v].y, dEPSILON)) {
      break;
    }

    if (next == v) /* So we cannot get into endless loop */
    {
      break;
    }

    next--;
  }

  dx0 = points_[next].x - points_[v].x;
  dy0 = points_[next].y - points_[v].y;

  /* following */
  next = v + 1;
  while (1) {
    if (next >= point_size_ - 1) {
      next = 0;
    }

    if (!IsEqual(points_[next].x, points_[v].x, dEPSILON) ||
        !IsEqual(points_[next].y, points_[v].y, dEPSILON)) {
      break;
    }

    if (next == v) /* So we cannot get into endless loop */
    {
      break;
    }

    next++;
  }

  dx1 = points_[next].x - points_[v].x;
  dy1 = points_[next].y - points_[v].y;

  crossproduct = dx1 * dy0 - dx0 * dy1;

  if (crossproduct > 0) /* CCW */
    return FALSE;
  else if (crossproduct < 0) /* CW */
    return TRUE;

  /* ok, this is a degenerate case : the extent of the polygon is less than
   * EPSILON */
  /* Try with Green Formula as a fallback, but this is not a guarantee */
  /* as we'll probably be affected by numerical instabilities */

  double dfSum =
      points_[0].x * (points_[1].y - points_[point_size_ - 1].y);

  for (i = 1; i < point_size_ - 1; i++) {
    dfSum += points_[i].x * (points_[i + 1].y - points_[i - 1].y);
  }

  dfSum += points_[point_size_ - 1].x *
           (points_[0].y - points_[point_size_ - 2].y);

  return dfSum < 0;
}

void LinearRing::ReverseWindingOrder() {
  int pos = 0;
  Point tempPoint;

  for (int i = 0; i < point_size_ / 2; i++) {
    GetPoint(i, &tempPoint);
    pos = point_size_ - i - 1;
    SetPoint(i, GetX(pos), GetY(pos));
    SetPoint(pos, tempPoint.GetX(), tempPoint.GetY());
  }
}

void LinearRing::CloseRings() {
  if (point_size_ < 2) return;

  if (GetX(0) != GetX(point_size_ - 1) || GetY(0) != GetY(point_size_ - 1))
    AddPoint(GetX(0), GetY(0));
}

double LinearRing::GetArea() const {
  return CalcPolygonArea(points_, point_size_);
}

bool LinearRing::IsPointInRing(const Point *poPoint, bool bTestEnvelope) const {
  if (NULL == poPoint) {
    return false;
  }

  const int iNumPoints = GetNumPoints();

  // Simple validation
  if (iNumPoints < 4) return 0;

  const double dfTestX = poPoint->GetX();
  const double dfTestY = poPoint->GetY();

  if (bTestEnvelope) {
    Envelope extent;
    GetEnvelope(&extent);
    if (!(dfTestX >= extent.MinX && dfTestX <= extent.MaxX &&
          dfTestY >= extent.MinY && dfTestY <= extent.MaxY)) {
      return false;
    }
  }

  return (PointToPolygon_New1(dbfPoint(poPoint->GetX(), poPoint->GetY()),
                              points_, point_size_) == -1);

  // const int iNumPoints = GetNumPoints();

  //// Simple validation
  // if ( iNumPoints < 4 )
  //	return 0;

  // const double dfTestX = poPoint->GetX();
  // const double dfTestY = poPoint->GetY();

  //// Fast test if point is inside extent of the ring
  // if (bTestEnvelope)
  //{
  //	Envelope extent;
  //	GetEnvelope(&extent);
  //	if ( !( dfTestX >= extent.MinX && dfTestX <= extent.MaxX
  //		&& dfTestY >= extent.MinY && dfTestY <= extent.MaxY ) )
  //	{
  //		return false;
  //	}
  //}

  //// For every point p in ring,
  //// test if ray starting from given point crosses segment (p - 1, p)
  // int iNumCrossings = 0;

  // for ( int iPoint = 1; iPoint < iNumPoints; iPoint++ )
  //{
  //	const int iPointPrev = iPoint - 1;

  //	const double x1 = GetX(iPoint) - dfTestX;
  //	const double y1 = GetY(iPoint) - dfTestY;

  //	const double x2 = GetX(iPointPrev) - dfTestX;
  //	const double y2 = GetY(iPointPrev) - dfTestY;

  //	if( ( ( y1 > 0 ) && ( y2 <= 0 ) ) || ( ( y2 > 0 ) && ( y1 <= 0 ) ) )
  //	{
  //		// Check if ray intersects with segment of the ring
  //		const double dfIntersection = ( x1 * y2 - x2 * y1 ) / (y2 - y1);
  //		if ( 0.0 < dfIntersection )
  //		{
  //			// Count intersections
  //			iNumCrossings++;
  //		}
  //	}
  //}

  //// If iNumCrossings number is even, given point is outside the ring,
  //// when the crossings number is odd, the point is inside the ring.
  // return ( ( iNumCrossings % 2 ) == 1 ? true : false );
}

bool LinearRing::IsPointOnRingBoundary(const Point *poPoint,
                                       bool bTestEnvelope) const {
  if (NULL == poPoint) {
    return 0;
  }

  const int iNumPoints = GetNumPoints();

  // Simple validation
  if (iNumPoints < 4) return 0;

  const double dfTestX = poPoint->GetX();
  const double dfTestY = poPoint->GetY();

  // Fast test if point is inside extent of the ring
  Envelope extent;
  GetEnvelope(&extent);
  if (!(dfTestX >= extent.MinX && dfTestX <= extent.MaxX &&
        dfTestY >= extent.MinY && dfTestY <= extent.MaxY)) {
    return 0;
  }

  return (PointToPolygon_New1(dbfPoint(poPoint->GetX(), poPoint->GetY()),
                              points_, point_size_) == 0);

  // for ( int iPoint = 1; iPoint < iNumPoints; iPoint++ )
  //{
  //	const int iPointPrev = iPoint - 1;

  //	const double x1 = GetX(iPoint) - dfTestX;
  //	const double y1 = GetY(iPoint) - dfTestY;

  //	const double x2 = GetX(iPointPrev) - dfTestX;
  //	const double y2 = GetY(iPointPrev) - dfTestY;

  //	/* If iPoint and iPointPrev are the same, go on */
  //	if (x1 == x2 && y1 == y2)
  //	{
  //		continue;
  //	}

  //	/* If the point is on the segment, return immediatly */
  //	/* FIXME? If the test point is not exactly identical to one of */
  //	/* the vertices of the ring, but somewhere on a segment, there's */
  //	/* little chance that we get 0. So that should be tested against some
  //epsilon */ 	if ( x1 * y2 - x2 * y1 == 0 )
  //	{
  //		return true;
  //	}
  //}

  // return false;
}
}  // namespace core