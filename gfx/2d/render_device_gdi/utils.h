// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_2D_RENDER_DEVICE_UTILS_H
#define GFX_2D_RENDER_DEVICE_UTILS_H

#include "base/base_config.h"

namespace gfx2d {
void ClearRect(HDC dc_, int x, int y, int w, int h,
               COLORREF clr = RGB(255, 255, 255));
void DrawRect(HDC dc_, RECT &rect, BOOL xor = TRUE);
void DrawLine(HDC dc_, POINT *points, int size, BOOL xor = TRUE);
void DrawCross(HDC dc_, long x, long y, long r, BOOL xor = TRUE);

long	GetImageTypeByFileExt(const char *file_name);

}  // namespace gfx2d

#endif  // GFX_2D_RENDER_DEVICE_UTILS_H