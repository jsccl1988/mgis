#ifndef GEOMETRY_BASE_RECT_H
#define GEOMETRY_BASE_RECT_H

#include "core/core.h"
#include "geometry/base/dot.h"
#include "geometry/export.h"

namespace core {
template <typename T>
struct Rect2D {
  Point2D<T> lb;
  Point2D<T> rt;

  Rect2D() { lb.x = rt.x = lb.y = rt.y = kDoubleMax; }

  ~Rect2D() {}

  bool IsInit() const {
    return (lb.x != kDoubleMax || lb.y != kDoubleMax || rt.x != kDoubleMax ||
            rt.y != kDoubleMax);
  }

  long Height(void) const { return std::abs(rt.y - lb.y); }
  long Width(void) const { return std::abs(rt.x - lb.x); }

  void Merge(T x, T y) {
    Point2D<T> tmp(0, 0);
    if (lb == tmp && rt == tmp) {
      lb = rt = Point2D<T>(x, y);
    } else {
      lb.x = std::min(lb.x, x);
      rt.x = std::max(rt.x, x);
      lb.y = std::min(lb.y, y);
      rt.y = std::max(rt.y, y);
    }
  }

  void Merge(const Rect2D& other) {
    if (IsInit() && other.IsInit()) {
      lb.x = min(lb.x, other.lb.x);
      rt.x = max(rt.x, other.rt.x);
      lb.y = min(lb.y, other.lb.y);
      rt.y = max(rt.y, other.rt.y);
    } else {
      lb.x = other.lb.x;
      rt.x = other.rt.x;
      lb.y = other.lb.y;
      rt.y = other.rt.y;
    }
  }
  void Merge(T x, T y) {
    if (IsInit()) {
      lb.x = min(lb.x, x);
      rt.x = max(rt.x, x);
      lb.y = min(lb.y, y);
      rt.y = max(rt.y, y);
    } else {
      lb.x = rt.x = x;
      lb.y = rt.y = y;
    }
  }

  void Intersect(const Rect2D& other) {
    if (Intersects(other)) {
      if (IsInit()) {
        lb.x = max(lb.x, other.lb.x);
        rt.x = min(rt.x, other.rt.x);
        lb.y = max(lb.y, other.lb.y);
        rt.y = min(rt.y, other.rt.y);
      } else {
        lb.x = other.lb.x;
        rt.x = other.rt.x;
        lb.y = other.lb.y;
        rt.y = other.rt.y;
      }
    } else {
      lb.x = 0;
      rt.x = 0;
      lb.y = 0;
      rt.y = 0;
    }
  }

  bool Intersects(Rect2D const& other) const {
    return lb.x <= other.rt.x && rt.x >= other.lb.x && lb.y <= other.rt.y &&
           rt.y >= other.lb.y;
  }

  bool Contains(Rect2D const& other) const {
    return lb.x <= other.lb.x && lb.y <= other.lb.y && rt.x >= other.rt.x &&
           rt.y >= other.rt.y;
  }

  bool Contains(double x, double y) const {
    return lb.x <= x && lb.y <= y && rt.x >= x && rt.y >= y;
  }
};
}  // namespace core

#endif  // GEOMETRY_BASE_RECT_H