// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

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

  int Init(HWND hwnd) override;
  int Destroy(void) override;
  int Release(void) override;

 public:
  int Resize(DRect rect) override;
  int Refresh(void) override;
  int Refresh(LRect rect);
  int RefreshDirectly(DRect rect, bool realtime = false) override;

  int ZoomMove(LPoint offset, bool realtime = false) override;
  int ZoomScale(LPoint original_point, float scale, bool realtime = false) override;
  int ZoomToRect(LRect rect, bool realtime = false) override;

 public:
  int LPToDP(float x, float y, long &X, long &Y) const override;
  int DPToLP(long X, long Y, float &x, float &y) const;
  int LRectToDRect(const LRect &lrect, DRect &drect) const override;
  int DRectToLRect(const DRect &drect, LRect &lrect) const override;

 public:
  int BeginRender(eRenderBuffer render_buffer, bool clear = false,
                  const Style *style = NULL, int op = R2_COPYPEN) override;
  int Render(void) override;
  int EndRender(eRenderBuffer render_buffer) override;
  int Swap(void) override;

  int RenderForDebug() override;
  int RenderLayer(const OGRLayer *layer, int op = R2_COPYPEN) override;
  int RenderFeature(const OGRFeature *feature, int op = R2_COPYPEN) override;
  int RenderGeometry(const OGRGeometry *geomtry, int op = R2_COPYPEN) override;

 public:
  int DrawMultiLineString(const OGRMultiLineString *multi_linestring) override;
  int DrawMultiPoint(const OGRMultiPoint *multi_point) override;
  int DrawMultiPolygon(const OGRMultiPolygon *multi_polygon) override;

  int DrawPoint(const OGRPoint *point) override;
  int DrawAnno(const OGRPoint *point, const char *anno, float angle,
               float height, float width, float space) override;
  int DrawSymbol(const OGRPoint *point, HICON icon, long height, long width) override;

  int DrawLineString(const OGRLineString *line_string) override;
  int DrawLinearRing(const OGRLinearRing *linear_ring) override;
  int DrawPolygon(const OGRPolygon *polygon) override;

 public:
  int DrawImage(const char *image_buffer, int image_buffer_size,
                const LRect &rect, long code_type,
                eRenderBuffer render_buffer = RB_COMPOSIT) override;
  int StrethImage(const char *image_buffer, int image_buffer_size,
                  const LRect &rect, long code_type,
                  eRenderBuffer render_buffer = RB_COMPOSIT) override;

 public:
  int SaveImage(const char *file_path,
                eRenderBuffer render_buffer = RB_COMPOSIT,
                bool backgroud_transparent = false) override;
  int Save2ImageBuffer(char *&image_buffer, long &image_buffer_size,
                       long code_type,
                       eRenderBuffer render_buffer = RB_COMPOSIT,
                       bool backgroud_transparent = false) override;
  int FreeImageBuffer(char *&image_buffer) override;

 protected:
  int PrepareForDrawing(const Style *style, int draw_mode = R2_COPYPEN);
  int EndDrawing();

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

  RenderBuffer composit_render_buffer_;
  RenderBuffer render_buffer_;
  int op_;

  OGRLayer *layer_;

  // For Debug
  std::string polygon_wkt_{
      "POLYGON((50 50, 2350 50, 2350 2350, 50 2350, 50 50))"};
  std::string text_anchor_wkt_{"POINT(1000 1000)"};

  OGRPolygon polygon_;
  OGRPoint text_anchor_;
  std::string text_{"smart gis"};
};
}  // namespace gfx2d

#endif  // GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H