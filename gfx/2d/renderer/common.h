// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_2D_RENDERER_COMMON_H
#define GFX_2D_RENDERER_COMMON_H

#include <windef.h>
#include <windows.h>
#include <wingdi.h>
#include <wtypes.h>

#include <vector>

#include "base/base_config.h"
#include "base/basictypes.h"
#include "base/error.h"
#include "base/macros.h"
#include "build/build_config.h"
#include "gfx/2d/renderer/render_export.h"
#include "ogrsf_frmts.h"


namespace gfx2d {
enum RHI2D { RHI2D_GDI, RHI2D_SKIA };
struct RenderOptions {
  bool show_mbr = true;
  bool show_point = true;
  long point_radius = 2;
};

template <typename T>
struct point_t {
  T x = T();
  T y = T();

  point_t() {}
  point_t(T _x, T _y) : x(_x), y(_y){}
};

template <typename T>
struct rect_t {
  T x = T();
  T y = T();
  T width = T();
  T height = T();
};

template <typename T>
struct size_t {
  T width = T();
  T height = T();
};

template <typename T1, typename T2>
bool equal(const rect_t<T1>& left, const rect_t<T2>& right) {
  return (EQUAL(left.x, right.x) && EQUAL(left.y, right.y) &&
          EQUAL(left.height, right.height) && EQUAL(left.width, right.width));
}

template <typename T1, typename T2>
bool equal(const point_t<T1>& left, const point_t<T2>& right) {
  return (EQUAL(left.x, right.x) && EQUAL(left.y, right.y));
}

template <typename T>
bool is_empty(const rect_t<T>& rect) {
  return (EQUAL(rect.width, 0) && EQUAL(rect.height, 0));
}

using DPoint = point_t<long>;
using LPoint = point_t<float>;

using DRect = rect_t<long>;
using LRect = rect_t<float>;

using DSize = size_t<long>;
using LSize = size_t<float>;

using Viewport = LRect;
using Windowport = LRect;
}  // namespace gfx2d

#endif  // GFX_2D_RENDERER_COMMON_H