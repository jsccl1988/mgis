#ifndef GFX_2D_UTILS_DIRECT_H
#define GFX_2D_UTILS_DIRECT_H

#include <wingdi.h>

namespace gfx2d {
void ClearRect(HDC hDC, int x, int y, int w, int h,
               COLORREF clr = RGB(255, 255, 255));
void DrawRect(HDC hDC, RECT &rect, BOOL xor = TRUE);
void DrawRect(HDC hDC, lRect &lrect, BOOL xor = TRUE);

void DrawLine(HDC hDC, lPoint *plPoints, int nCount, BOOL xor = TRUE);
void DrawLine(HDC hDC, POINT *pPoints, int nCount, BOOL xor = TRUE);

void DrawCross(HDC hDC, long lX, long lY, long r, BOOL xor = TRUE);
}  // namespace gfx2d

#endif  // GFX_2D_UTILS_DIRECT_H