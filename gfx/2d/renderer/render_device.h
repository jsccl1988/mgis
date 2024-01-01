#ifndef GFX_2D_RENDERER_RENDER_DEVICE_H
#define GFX_2D_RENDERER_RENDER_DEVICE_H

#include "gfx/2d/renderer/common.h"
#include "gfx/2d/renderer/style.h"
#include "ogrsf_frmts.h"


namespace gfx2d {
class RenderDevice {
 public:
  enum eRenderBufferLayer { RB_COMPOSIT, RB_DIRECT };
  RenderDevice(HINSTANCE instance)
      : rhi_api_(RHI2D_GDI),
        instance_handle_(instance),
        hwnd_(nullptr),
        map_mode_(MM_TEXT),
        blc_(1.) {
    ;
  }

  virtual ~RenderDevice(void) {}

 public:
  virtual int Init(HWND hwnd) = 0;
  virtual int Destroy(void) = 0;
  virtual int Release(void) = 0;

  void SetViewport(const Viewport &viewport) { viewport_ = viewport; }
  Viewport GetViewport(void) const { return viewport_; }

  void SetWindowport(const Windowport &windowport) { windowport_ = windowport; }
  Windowport GetWindowport(void) const { return windowport_; }

  void SetCurrentDOP(const LPoint &point) { current_dop_ = point; }
  LPoint GetCurrentDOP(void) const { return current_dop_; }

  void SetMapMode(int mode) { map_mode_ = mode; }
  int GetMapMode(void) const { return map_mode_; }

  void SetRenderOptions(const RenderOptions &options) { options_ = options; }
  RenderOptions GetRenderOptions(void) const { return options_; }

  inline float GetBLC(void) const { return blc_; }

 public:
  virtual int Resize(DRect rect) = 0;
  virtual int Refresh(void) = 0;
  virtual int Refresh(LRect rect) = 0;
  virtual int RefreshDirectly(DRect rect, bool realtime = false) = 0;

  virtual int ZoomMove(LPoint offset, bool realtime = false) = 0;
  virtual int ZoomScale(LPoint original_point, float scale,
                        bool realtime = false) = 0;
  virtual int ZoomToRect(LRect rect, bool realtime = false) = 0;

  virtual int Timer() = 0;

 public:
  virtual int LPToDP(float x, float y, long &X, long &Y) const = 0;
  virtual int DPToLP(long X, long Y, float &x, float &y) const = 0;
  virtual int LRectToDRect(const LRect &rect, DRect &lrect) const = 0;
  virtual int DRectToLRect(const DRect &lrect, LRect &rect) const = 0;

 public:
  virtual int BeginRender(eRenderBufferLayer rbl_type, bool clear = false,
                          const Style *style = nullptr,
                          int op = R2_COPYPEN) = 0;
  virtual int EndRender(eRenderBufferLayer rbl_type) = 0;
  virtual int Render(void) = 0;

 public:
  virtual int RenderLayer(const OGRLayer *layer, int op = R2_COPYPEN) = 0;
  virtual int RenderFeature(const OGRFeature *feature, int op = R2_COPYPEN) = 0;
  virtual int RenderGeometry(const OGRGeometry *geomtry,
                             int op = R2_COPYPEN) = 0;

  virtual int DrawMultiLineString(
      const OGRMultiLineString *multi_linestring) = 0;
  virtual int DrawMultiPoint(const OGRMultiPoint *multi_point) = 0;
  virtual int DrawMultiPolygon(const OGRMultiPolygon *multi_polygon) = 0;

  virtual int DrawPoint(const OGRPoint *point) = 0;
  virtual int DrawAnno(const OGRPoint *point, const char *anno, float angle,
                       float height, float width, float space) = 0;
  virtual int DrawSymbol(const OGRPoint *point, HICON icon, long height,
                         long width) = 0;

  virtual int DrawLineString(const OGRLineString *line_string) = 0;
  virtual int DrawLinearRing(const OGRLinearRing *linear_ring) = 0;

  virtual int DrawPolygon(const OGRPolygon *polygon) = 0;

 public:
  virtual int DrawImage(const char *image_buffer, int image_buffer_size,
                        const LRect &rect, long code_type_,
                        eRenderBufferLayer rbl_type = RB_COMPOSIT) = 0;
  virtual int StrethImage(const char *image_buffer, int image_buffer_size,
                          const LRect &rect, long code_type_,
                          eRenderBufferLayer rbl_type = RB_COMPOSIT) = 0;

 public:
  virtual int SaveImage(const char *file_path,
                        eRenderBufferLayer rbl_type = RB_COMPOSIT,
                        bool backgroud_transparent = false) = 0;
  virtual int Save2ImageBuffer(char *&image_buffer, long &image_buffer_size,
                               long code_type_,
                               eRenderBufferLayer rbl_type = RB_COMPOSIT,
                               bool backgroud_transparent = false) = 0;
  virtual int FreeImageBuffer(char *&image_buffer) = 0;

 protected:
  HINSTANCE instance_handle_;
  RHI2D rhi_api_;

  Viewport viewport_;
  Windowport windowport_;
  float blc_;

  HWND hwnd_;
  int map_mode_;
  RenderOptions options_;

  LPoint current_dop_;
};

using HRENDERDEVICE = RenderDevice *;

#ifdef __cplusplus
extern "C" {
#endif
int GFX_2D_RENDERER_EXPORT CreateRenderDevice(HINSTANCE instance,
                                              HRENDERDEVICE &render_device);
int GFX_2D_RENDERER_EXPORT DestroyRenderDevice(HRENDERDEVICE &render_device);

using CreateRenderDeviceFn = HRESULT (*)(HINSTANCE instance,
                                         HRENDERDEVICE &render_device);
using DestroyRenderDeviceFn = HRESULT (*)(HRENDERDEVICE &render_device);

#ifdef __cplusplus
}
#endif
}  // namespace gfx2d

#endif  // GFX_2D_RENDERER_RENDER_DEVICE_H