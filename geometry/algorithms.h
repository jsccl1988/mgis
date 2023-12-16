#ifndef GEOMETRY_ALGORITHMS_H
#define GEOMETRY_ALGORITHMS_H

#include <math.h>

#include "core/core.h"
#include "geometry/base/dot.h"
#include "geometry/base/matrix2d.h"
#include "geometry/base/rect.h"
#include "geometry/base/vector2d.h"
#include "geometry/export.h"

namespace core {
using RawPoint = Dot2D<double>;
using Envelope = Rect2D<double>;
long GEOMETRY_EXPORT GetEnvlope(Envelope &env, RawPoint *pPoints, int nPoint);

double GEOMETRY_EXPORT Distance(double x1, double y1, double x2, double y2);
double GEOMETRY_EXPORT Distance(RawPoint &pt1, RawPoint *pPoints, int nPoint,
                                int &indexPre, int &indexNext);
double GEOMETRY_EXPORT Distance(RawPoint &pt1, std::vector<RawPoint> &vPoints,
                                int &indexPre, int &indexNext);

double GEOMETRY_EXPORT Distance_New(RawPoint &pt1, RawPoint *pPoints,
                                    int nPoint, int &indexPre, int &indexNext);
double GEOMETRY_EXPORT Distance_New(RawPoint &pt1,
                                    std::vector<RawPoint> &vPoints,
                                    int &indexPre, int &indexNext);

long GEOMETRY_EXPORT Locate(RawPoint &pt1, RawPoint *pPoints, int nPoint,
                            int &indexPre, int &indexNext);
long GEOMETRY_EXPORT Locate(RawPoint &pt1, std::vector<RawPoint> &vPoints,
                            int &indexPre, int &indexNext);

long GEOMETRY_EXPORT PointToLineSect(RawPoint &pt1, RawPoint &ptA,
                                     RawPoint &ptB);

long GEOMETRY_EXPORT PointToPolygon(RawPoint &pt1, RawPoint *pPoints,
                                    int nPoint);

long GEOMETRY_EXPORT PointToPolygon_New(RawPoint &pt1, RawPoint *pPoints,
                                        int nPoint);
long GEOMETRY_EXPORT PointToPolygon_New1(RawPoint &pt1, RawPoint *pPoints,
                                         int nPoint);

double GEOMETRY_EXPORT CalcPolygonArea(RawPoint *pPoints, int nPoint);

double GEOMETRY_EXPORT CalcTriArea(RawPoint A, RawPoint B, RawPoint C);

int GEOMETRY_EXPORT GetH(RawPoint A, RawPoint B, RawPoint P, RawPoint &H);

double GEOMETRY_EXPORT CosABC(RawPoint A, RawPoint B, RawPoint C);

long GEOMETRY_EXPORT Interpolate_Lugrange(RawPoint *&pPnts, int &nPnts,
                                          RawPoint *pCtrlPnts, int nCtrlPnts,
                                          int nSection = 50);
long GEOMETRY_EXPORT Interpolate_Geometry(RawPoint *&pPnts, int &nPnts,
                                          RawPoint *pCtrlPnts, int nCtrlPnts,
                                          float dt = 0.02);
long GEOMETRY_EXPORT Interpolate_BSpline(RawPoint *&pPnts, int &nPnts,
                                         RawPoint *pCtrlPnts, int nCtrlPnts,
                                         float dt = 0.02);
long GEOMETRY_EXPORT Interpolate_3Spline(RawPoint *&pPnts, int &nPnts,
                                         RawPoint *pCtrlPnts, int nCtrlPnts,
                                         int nInterCount = 50);

long GEOMETRY_EXPORT DivPolygenIntoTriMesh(vector<Triangle> &trilist,
                                           RawPoint *pPoints, int nPoint);
}  // namespace core
#endif  // GEOMETRY_ALGORITHMS_H
