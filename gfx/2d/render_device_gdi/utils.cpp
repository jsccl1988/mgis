#include "gfx/2d/render_device_gdi/utils.h"

#include <wingdi.h>

namespace gfx2d {
void ClearRect(HDC dc_, int x, int y, int w, int h, COLORREF clr) {
  RECT rect;
  rect.left = x;
  rect.bottom = y;
  rect.right = x + w;
  rect.top = y + h;

  HBRUSH hBrush = ::CreateSolidBrush(clr);
  HBRUSH hOldBrush = (HBRUSH)::SelectObject(dc_, hBrush);
  ::FillRect(dc_, &rect, hBrush);
  ::SelectObject(dc_, hOldBrush);
  ::DeleteObject(hBrush);
}

void DrawRect(HDC dc_, RECT &rect, BOOL xor) {
  ::MoveToEx(dc_, rect.left, rect.top, NULL);
  ::LineTo(dc_, rect.right, rect.top);
  ::LineTo(dc_, rect.right, rect.bottom);
  ::LineTo(dc_, rect.left, rect.bottom);
  ::LineTo(dc_, rect.left, rect.top);
}

void DrawLine(HDC dc_, POINT *points, int size, BOOL xor) {
  if (points == NULL) return;

  ::MoveToEx(dc_, points[0].x, points[0].y, NULL);
  ::PolylineTo(dc_, points, size);
}

void DrawCross(HDC dc_, long x, long y, long r, BOOL xor) {
  ::MoveToEx(dc_, x - r, y, NULL);
  ::LineTo(dc_, x + r, y);

  ::MoveToEx(dc_, x, y, NULL);
  ::LineTo(dc_, x, y);
}
}  // namespace gfx2d
