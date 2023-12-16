#ifndef GFX_2D_RENDERER_H
#define GFX_2D_RENDERER_H

#include "gfx/2d/renderer/export.h"

#include <wtypes.h>

namespace gfx2d {
enum RHI2D { RHI2D_GDI };
struct RenderOptions {
  bool show_mbr = true;
  bool show_point = true;
  long point_raduis = 2;
};
class GFX_2D_RENDERER_EXPORT Renderer {
 public:
  Renderer(HINSTANCE hInst);
  ~Renderer(void);

  int CreateDevice(const char* chAPI);
  LPRENDERDEVICE GetDevice(void) { return m_pDevice; }
  void Release(void);

 private:
  LPRENDERDEVICE m_pDevice;
  HINSTANCE m_hInst;
  HMODULE dll_;
};

typedef Renderer* LPRENDERER;
}  // namespace gfx2d

#endif  //GFX_2D_RENDERER_H