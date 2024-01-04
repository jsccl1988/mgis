#include "gfx/2d/render_device_gdi/render_buffer.h"

#include "base/util/string_util.h"
#include "gfx/2d/render_device_gdi/utils.h"
#include "third_party/CxImage/CxImage/ximage.h"

#pragma comment(lib, "msimg32.lib")  // TransparentBlt

namespace gfx2d {
RenderBuffer::RenderBuffer(HWND hwnd) : hwnd_(hwnd), owned_(true) {}
RenderBuffer::RenderBuffer(void) : hwnd_(NULL), owned_(true) {}

RenderBuffer::~RenderBuffer(void) {
  if (owned_ && paint_buffer_) {
    ::DeleteObject(paint_buffer_);
    paint_buffer_ = NULL;
  }
}

inline long RenderBuffer::SetHWND(HWND hwnd) {
  if (!owned_) {
    return ERR_FAILURE;
  }
  hwnd_ = hwnd;
  return ERR_NONE;
}

long RenderBuffer::SetSize(int cx, int cy) {
  if (!owned_ || cx < 1 || cy < 1) {
    return ERR_FAILURE;
  }

  if (paint_buffer_) {
    ::DeleteObject(paint_buffer_);
    paint_buffer_ = NULL;
  }

  HDC dc_ = ::GetDC(hwnd_);
  width_ = cx;
  height_ = cy;
  paint_buffer_ = ::CreateCompatibleBitmap(dc_, width_, height_);

  if (NULL == paint_buffer_) {
    return ERR_FAILURE;
  }

  Clear(0, 0, width_, height_);

  ::ReleaseDC(hwnd_, dc_);

  return ERR_NONE;
}

long RenderBuffer::ShareBuffer(RenderBuffer &source_render_buffer) {
  if (owned_ && paint_buffer_) {
    ::DeleteObject(paint_buffer_);
    paint_buffer_ = NULL;
  }

  this->SetHWND(source_render_buffer.GetHWND());
  this->height_ = source_render_buffer.GetHeight();
  this->width_ = source_render_buffer.GetWidth();
  this->paint_buffer_ = source_render_buffer.GetBuffer();

  owned_ = false;

  return ERR_NONE;
}

long RenderBuffer::Clear(int x, int y, int w, int h, COLORREF clr) {
  if (w < 1 || h < 1) {
    return ERR_FAILURE;
  }

  RECT rect;
  rect.left = x;
  rect.bottom = y;
  rect.right = x + w;
  rect.top = y + h;

  HDC dc_ = ::GetDC(hwnd_);
  HDC paint_buffer_dc = ::CreateCompatibleDC(dc_);
  HBITMAP pre_paint_buffer =
      (HBITMAP)::SelectObject(paint_buffer_dc, paint_buffer_);

  HBRUSH brush = ::CreateSolidBrush(clr);
  HBRUSH old_brush = (HBRUSH)::SelectObject(paint_buffer_dc, brush);

  ::FillRect(paint_buffer_dc, &rect, brush);

  ::SelectObject(paint_buffer_dc, old_brush);
  ::DeleteObject(brush);

  ::SelectObject(paint_buffer_dc, pre_paint_buffer);
  ::DeleteObject(paint_buffer_dc);

  ::ReleaseDC(hwnd_, dc_);

  return ERR_NONE;
}

long RenderBuffer::Swap(int dest_org_x, int dest_org_y, int dest_w, int dest_h,
                        int src_org_x, int src_org_y, int op) {
  if (paint_buffer_ == NULL) {
    return ERR_FAILURE;
  }

  HDC dc_ = ::GetDC(hwnd_);
  HDC source_dc_ = this->PrepareDC(false);

  BOOL ret = ::BitBlt(dc_, dest_org_x, dest_org_y, dest_w, dest_h, source_dc_,
                      src_org_x, src_org_y, op);

  this->EndDC();
  ::ReleaseDC(hwnd_, dc_);

  if (ret) {
    return ERR_NONE;
  } else {
    return ERR_FAILURE;
  }
}

long RenderBuffer::Swap(int dest_org_x, int dest_org_y, int dest_w, int dest_h,
                        int src_org_x, int src_org_y, int src_w, int src_h,
                        eSwapType type, int op, COLORREF clr) {
  if (paint_buffer_ == NULL) {
    return ERR_FAILURE;
  }

  HDC dc_ = ::GetDC(hwnd_);
  HDC source_dc_ = this->PrepareDC(false);
  BOOL ret = TRUE;

  switch (type) {
    case BLT_STRETCH:
      ret = ::StretchBlt(dc_, dest_org_x, dest_org_y, dest_w, dest_h,
                         source_dc_, src_org_x, src_org_y, src_w, src_h, op);
      break;
    case BLT_TRANSPARENT:
      ret = ::TransparentBlt(dc_, dest_org_x, dest_org_y, dest_w, dest_h,
                             source_dc_, src_org_x, src_org_y, dest_w, dest_h,
                             clr);
      break;
  }

  this->EndDC();
  ::ReleaseDC(hwnd_, dc_);

  if (ret) {
    return ERR_NONE;
  } else {
    return ERR_FAILURE;
  }
}

long RenderBuffer::Swap(RenderBuffer &target_render_buffer, int dest_org_x,
                        int dest_org_y, int dest_w, int dest_h, int src_org_x,
                        int src_org_y, int op) {
  if (paint_buffer_ == NULL) {
    return ERR_FAILURE;
  }

  HDC target_dc = target_render_buffer.PrepareDC(false);
  HDC source_dc_ = this->PrepareDC(false);

  BOOL ret = ::BitBlt(target_dc, dest_org_x, dest_org_y, dest_w, dest_h,
                      source_dc_, src_org_x, src_org_y, op);

  this->EndDC();
  target_render_buffer.EndDC();

  if (ret) {
    return ERR_NONE;
  } else {
    return ERR_FAILURE;
  }
}

long RenderBuffer::Swap(RenderBuffer &target_render_buffer, int dest_org_x,
                        int dest_org_y, int dest_w, int dest_h, int src_org_x,
                        int src_org_y, int src_w, int src_h, eSwapType type,
                        int op, COLORREF clr) {
  if (paint_buffer_ == NULL) {
    return ERR_FAILURE;
  }

  HDC target_dc = target_render_buffer.PrepareDC(false);
  HDC source_dc_ = this->PrepareDC(false);

  BOOL ret = TRUE;

  switch (type) {
    case BLT_STRETCH:
      ret = ::StretchBlt(target_dc, dest_org_x, dest_org_y, dest_w, dest_h,
                         source_dc_, src_org_x, src_org_y, src_w, src_h, op);
      break;
    case BLT_TRANSPARENT:
      ret =
          ::TransparentBlt(target_dc, dest_org_x, dest_org_y, dest_w, dest_h,
                           source_dc_, src_org_x, src_org_y, src_w, src_h, clr);
      break;
  }

  this->EndDC();
  target_render_buffer.EndDC();

  if (ret)
    return ERR_NONE;
  else
    return ERR_FAILURE;
}

HDC RenderBuffer::PrepareDC(bool is_clip) {
  HDC dc_ = ::GetDC(hwnd_);
  paint_dc_ = ::CreateCompatibleDC(dc_);
  ::ReleaseDC(hwnd_, dc_);

  old_paint_buffer_ = (HBITMAP)::SelectObject(paint_dc_, paint_buffer_);

  if (is_clip) {
    RECT clip_rect;
    HRGN new_rgn = ::CreateRectRgn(0, 0, width_, height_);
    ::GetClipBox(paint_dc_, &clip_rect);
    ::SelectClipRgn(paint_dc_, new_rgn);
    ::DeleteObject(new_rgn);
  }

  return paint_dc_;
}

long RenderBuffer::EndDC(void) {
  if (paint_dc_) {
    ::SelectObject(paint_dc_, old_paint_buffer_);
    ::DeleteDC(paint_dc_);

    paint_dc_ = NULL;
  }

  return ERR_NONE;
}

RenderBuffer &RenderBuffer::operator=(const RenderBuffer &other) {
  if (&other == this) return *this;

  if (owned_ && NULL != paint_buffer_) {
    ::DeleteObject(paint_buffer_);
    paint_buffer_ = NULL;
  }

  this->SetHWND(other.GetHWND());
  this->SetSize(other.GetWidth(), other.GetHeight());
  return *this;
}

long RenderBuffer::DrawImage(const char *image_buffer, int image_buffer_size,
                             long code_type, long x, long y, long cx, long cy) {
  if (NULL == image_buffer || 0 == image_buffer_size) {
    return ERR_FAILURE;
  }

  CxImage image;
  image.Decode((BYTE *)image_buffer, image_buffer_size, code_type);

  HDC dc_ = PrepareDC();
  image.Draw(dc_, x, y, cx, cy);
  EndDC();

  return ERR_NONE;
}

long RenderBuffer::StrethImage(const char *image_buffer, int image_buffer_size,
                               long code_type, long xoffset, long yoffset,
                               long xsize, long ysize, DWORD rop) {
  if (NULL == image_buffer || 0 == image_buffer_size) {
    return ERR_FAILURE;
  }

  CxImage image;
  image.Decode((BYTE *)image_buffer, image_buffer_size, code_type);

  HDC dc_ = PrepareDC();
  image.Stretch(dc_, xoffset, yoffset, xsize, ysize, rop);
  EndDC();

  return ERR_NONE;
}

long RenderBuffer::Save2Image(const char *file_path,
                              bool backgroud_transparent) {
  long ret = Save2Image(paint_buffer_, file_path, backgroud_transparent);
  return ret;
}

long RenderBuffer::Save2ImageBuffer(char *&image_buffer,
                                    long &image_buffer_size, long code_type,
                                    bool backgroud_transparent) {
  long ret = Save2ImageBuffer(paint_buffer_, image_buffer, image_buffer_size,
                              code_type, backgroud_transparent);
  return ret;
}

long RenderBuffer::Save2Image(HBITMAP bitmap, const char *file_path,
                              bool backgroud_transparent) {
  if (bitmap == NULL || strlen(file_path) == 0) {
    return ERR_FAILURE;
  }

  CxImage image;
  if (image.CreateFromHBITMAP(bitmap)) {
    if (backgroud_transparent) {
      COLORREF bgClr = RGB(255, 255, 255);
      RGBQUAD transClr;
      transClr.rgbRed = GetRValue(bgClr);
      transClr.rgbGreen = GetGValue(bgClr);
      transClr.rgbBlue = GetBValue(bgClr);
      transClr.rgbReserved = 0;

      image.SetTransIndex(0);
      image.SetTransColor(transClr);
    }

    if (image.Save(file_path, GetImageTypeByFileExt(file_path))) {
      return ERR_NONE;
    }
  }

  return ERR_FAILURE;
}

long RenderBuffer::Save2ImageBuffer(HBITMAP bitmap, char *&image_buffer,
                                    long &image_buffer_size, long code_type,
                                    bool backgroud_transparent) {
  if (bitmap == NULL || image_buffer != NULL) {
    return ERR_FAILURE;
  }

  BYTE *buffer = NULL;
  int32_t size = 0;
  CxImage image;

  if (image.CreateFromHBITMAP(bitmap)) {
    if (backgroud_transparent) {
      COLORREF bgClr = RGB(255, 255, 255);
      RGBQUAD transClr;
      transClr.rgbRed = GetRValue(bgClr);
      transClr.rgbGreen = GetGValue(bgClr);
      transClr.rgbBlue = GetBValue(bgClr);
      transClr.rgbReserved = 0;

      image.SetTransIndex(0);
      image.SetTransColor(transClr);
    }

    if (image.Encode(buffer, size, code_type)) {
      image_buffer = (char *)buffer;
      image_buffer_size = size;

      return ERR_NONE;
    }

    return ERR_NONE;
  }

  return ERR_FAILURE;
}

long RenderBuffer::FreeImageBuffer(char *&image_buffer) {
  SAFE_DELETE_A(image_buffer);
  return ERR_NONE;
}
}  // namespace gfx2d