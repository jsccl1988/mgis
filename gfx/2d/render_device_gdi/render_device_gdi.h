#ifndef GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H
#define GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H

#include <string>

#include "base/base_config.h"
#include "build/build_config.h"
#include "gfx/2d/render_device_gdi/render_buffer.h"
#include "gfx/2d/renderer/render_device.h"

namespace gfx2d {
class RenderDeviceGDI : public RenderDevice {
 public:
  RenderDeviceGDI(HINSTANCE instance);
  virtual ~RenderDeviceGDI(void);

  int Init(HWND hwnd);
  int Destroy(void);
  int Release(void);

 public:
  int Resize(DRect rect);
  int Refresh(void);
  int Refresh(LRect rect);
  int RefreshDirectly(DRect rect, bool realtime = false);

  int ZoomMove(LPoint offset, bool realtime = false);
  int ZoomScale(LPoint original_point, float scale, bool realtime = false);
  int ZoomToRect(LRect rect, bool realtime = false);

  int Timer();

 public:
  int LPToDP(float x, float y, long &X, long &Y) const;
  int DPToLP(long X, long Y, float &x, float &y) const;
  int LRectToDRect(const LRect &lrect, DRect &drect) const;
  int DRectToLRect(const DRect &drect, LRect &lrect) const;

 public:
  int BeginRender(eRenderBuffer render_buffer, bool clear = false,
                  const Style *style = NULL, int op = R2_COPYPEN);
  int EndRender(eRenderBuffer render_buffer);
  int Render(void);

  int RenderLayer(const OGRLayer *layer, int op = R2_COPYPEN);
  int RenderFeature(const OGRFeature *feature, int op = R2_COPYPEN);
  int RenderGeometry(const OGRGeometry *geomtry, int op = R2_COPYPEN);

 public:
  int DrawMultiLineString(const OGRMultiLineString *multi_linestring);
  int DrawMultiPoint(const OGRMultiPoint *multi_point);
  int DrawMultiPolygon(const OGRMultiPolygon *multi_polygon);

  int DrawPoint(const OGRPoint *point);
  int DrawAnno(const OGRPoint *point, const char *anno, float angle,
               float height, float width, float space);
  int DrawSymbol(const OGRPoint *point, HICON icon, long height, long width);

  int DrawLineString(const OGRLineString *line_string);
  int DrawLinearRing(const OGRLinearRing *linear_ring);
  int DrawPolygon(const OGRPolygon *polygon);

 public:
  int DrawImage(const char *image_buffer, int image_buffer_size,
                const LRect &rect, long code_type,
                eRenderBuffer render_buffer = RB_COMPOSIT);
  int StrethImage(const char *image_buffer, int image_buffer_size,
                  const LRect &rect, long code_type,
                  eRenderBuffer render_buffer = RB_COMPOSIT);

 public:
  int SaveImage(const char *file_path,
                eRenderBuffer render_buffer = RB_COMPOSIT,
                bool backgroud_transparent = false);
  int Save2ImageBuffer(char *&image_buffer, long &image_buffer_size,
                       long code_type,
                       eRenderBuffer render_buffer = RB_COMPOSIT,
                       bool backgroud_transparent = false);
  int FreeImageBuffer(char *&image_buffer);

 protected:
  int PrepareForDrawing(const Style *style, int draw_mode = R2_COPYPEN);
  int EndDrawing();
  int Redraw();

 protected:
  HFONT font_;
  HPEN pen_;
  HBRUSH brush_;
  HICON icon_;

  HFONT old_font_;
  HPEN old_pen_;
  HBRUSH old_brush_;

  HDC current_dc_;
  bool use_current_style_;
  bool is_lock_style_;

  std::string anno_name_;
  float anno_angle_;

  Viewport zoomin_viewport_;
  Viewport zoomout_viewport_;

  LONGLONG last_redraw_command_stamp_;
  LONGLONG last_redraw_stamp_;
  bool is_redraw_;

  RenderBuffer composit_render_buffer_;
  RenderBuffer render_buffer_;
  int op_;

  OGRLayer *layer_;
};
}  // namespace gfx2d

#endif  // GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H