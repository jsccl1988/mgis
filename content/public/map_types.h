// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_PUBLIC_MAP_TYPES_H
#define CONTENT_PUBLIC_MAP_TYPES_H

#include "base/basictypes.h"

namespace content {
struct Point {
  int32_t x;
  int32_t y;
};

struct Rect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

struct Size {
  int32_t width;
  int32_t height;
};
}  // namespace content
#endif  // CONTENT_PUBLIC_MAP_TYPES_H
