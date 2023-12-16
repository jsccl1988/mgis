#include "geometry/curve/curve.h"

#include "geometry/algorithms.h"

namespace core {
Curve::Curve() {}
Curve::~Curve() {}

bool Curve::IsClosed() const {
  Point oStartPoint, oEndbfPoint;

  StartPoint(&oStartPoint);
  EndPoint(&oEndbfPoint);

  if (oStartPoint.GetX() == oEndbfPoint.GetX() &&
      oStartPoint.GetY() == oEndbfPoint.GetY()) {
    return true;
  } else {
    return false;
  }
}
}  // namespace core
