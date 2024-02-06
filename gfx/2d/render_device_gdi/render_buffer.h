// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_2D_RENDER_DEVICE_RENDER_BUFFER_H
#define GFX_2D_RENDER_DEVICE_RENDER_BUFFER_H

#include "base/base_config.h"
#include "build/build_config.h"

#include "gfx/2d/renderer/common.h"

namespace gfx2d {
class RenderBuffer {
 public:
  enum eSwapType {
    BLT_STRETCH,
    BLT_TRANSPARENT,
  };

  RenderBuffer(void);
  RenderBuffer(HWND hwnd);
  ~RenderBuffer(void);
  RenderBuffer &operator=(const RenderBuffer &other);

 public:
  inline HWND GetHWND() const { return hwnd_; }
  inline bool SetHWND(HWND hwnd);

  bool SetSize(int cx, int cy);
  inline long GetWidth(void) const { return width_; }
  inline long GetHeight(void) const { return height_; }

  inline HBITMAP GetBuffer(void) { return paint_buffer_; }
  bool ShareBuffer(RenderBuffer &source_render_buffer);

  bool Swap(int dest_org_x, int dest_org_y, int dest_w, int dest_h,
            int src_org_x, int src_org_y, int op = SRCCOPY);
  bool Swap(int dest_org_x, int dest_org_y, int dest_w, int dest_h,
            int src_org_x, int src_org_y, int src_w, int src_h,
            eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
            COLORREF clr = RGB(255, 255, 255));

  bool Swap(RenderBuffer &target_render_buffer, int dest_org_x, int dest_org_y,
            int dest_w, int dest_h, int src_org_x, int src_org_y,
            int op = SRCCOPY);
  bool Swap(RenderBuffer &target_render_buffer, int dest_org_x, int dest_org_y,
            int dest_w, int dest_h, int src_org_x, int src_org_y, int src_w,
            int src_h, eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
            COLORREF clr = RGB(255, 255, 255));

  bool Clear(int x, int y, int w, int h, COLORREF clr = RGB(255, 255, 255));

  HDC PrepareDC(bool is_clip = true);
  bool EndDC(void);

  bool DrawImage(const char *image_buffer, int image_buffer_size,
                 long codec, long x = 0, long y = 0, long cx = -1,
                 long cy = -1);
  bool StrethImage(const char *image_buffer, int image_buffer_size,
                   long codec, long xoffset, long yoffset, long xsize,
                   long ysize, DWORD rop = SRCCOPY);

  bool Save2Image(const char *file_path, bool backgroud_transparent = false);
  bool Save2ImageBuffer(char *&image_buffer, long &image_buffer_size,
                        long codec, bool backgroud_transparent = false);

  static bool Save2Image(HBITMAP bitmap, const char *file_path,
                         bool backgroud_transparent = false);
  static bool Save2ImageBuffer(HBITMAP bitmap, char *&image_buffer,
                               long &image_buffer_size, long codec,
                               bool backgroud_transparent = false);
  static bool FreeImageBuffer(char *&image_buffer);

 protected:
  HWND hwnd_;
  HDC paint_dc_;
  HBITMAP old_paint_buffer_;
  HBITMAP paint_buffer_;

  long width_;
  long height_;
  bool owned_;
};
}  // namespace gfx2d
#endif  // GFX_2D_RENDER_DEVICE_RENDER_BUFFER_H