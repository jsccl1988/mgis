// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#include "gfx/2d/render_device_gdi/utils.h"

#include <wingdi.h>

#include <string>

#include "third_party/CxImage/CxImage/ximage.h"

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

long GetImageTypeByFileExt(const char *file_name) {
  long image_type = CXIMAGE_FORMAT_UNKNOWN;

  std::string file = strlwr((char *)file_name);
  int pos = file.rfind('.');
  if (pos == std::string::npos) return CXIMAGE_FORMAT_UNKNOWN;

  std::string ext = file.substr(pos + 1, file.size());

  if (stricmp(ext.c_str(), "bmp") == 0)
    image_type = CXIMAGE_FORMAT_BMP;
  else if (stricmp(ext.c_str(), "gif") == 0)
    image_type = CXIMAGE_FORMAT_GIF;
  else if (stricmp(ext.c_str(), "jpg") == 0)
    image_type = CXIMAGE_FORMAT_JPG;
  else if (stricmp(ext.c_str(), "png") == 0)
    image_type = CXIMAGE_FORMAT_PNG;
  else if (stricmp(ext.c_str(), "mng") == 0)
    image_type = CXIMAGE_FORMAT_MNG;
  else if (stricmp(ext.c_str(), "ico") == 0)
    image_type = CXIMAGE_FORMAT_ICO;
  else if (stricmp(ext.c_str(), "tif") == 0)
    image_type = CXIMAGE_FORMAT_TIF;
  else if (stricmp(ext.c_str(), "tga") == 0)
    image_type = CXIMAGE_FORMAT_TGA;
  else if (stricmp(ext.c_str(), "pcx") == 0)
    image_type = CXIMAGE_FORMAT_PCX;
  else if (stricmp(ext.c_str(), "wbmp") == 0)
    image_type = CXIMAGE_FORMAT_WBMP;
  else if (stricmp(ext.c_str(), "wmf") == 0)
    image_type = CXIMAGE_FORMAT_WMF;
  /*else if (stricmp(ext.c_str(),"j2k") == 0)
          image_type = CXIMAGE_FORMAT_J2K;
  else if (stricmp(ext.c_str(),"jbg") == 0)
          image_type = CXIMAGE_FORMAT_JBG;*/
  else if (stricmp(ext.c_str(), "jpc") == 0)
    image_type = CXIMAGE_FORMAT_JPC;
  else if (stricmp(ext.c_str(), "jp2") == 0)
    image_type = CXIMAGE_FORMAT_JP2;
  else if (stricmp(ext.c_str(), "pgx") == 0)
    image_type = CXIMAGE_FORMAT_PGX;
  else if (stricmp(ext.c_str(), "pnm") == 0)
    image_type = CXIMAGE_FORMAT_PNM;
  else if (stricmp(ext.c_str(), "ras") == 0)
    image_type = CXIMAGE_FORMAT_RAS;

  return image_type;
}
}  // namespace gfx2d
