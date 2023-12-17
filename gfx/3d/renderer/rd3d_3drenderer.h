#ifndef _RD3D_RENDERER_H
#define _RD3D_RENDERER_H
#include "rd3d_3drenderdevice.h"

namespace _3Drd {
class CORE_EXPORT 3DRenderer {
 public:
  3DRenderer(HINSTANCE hInst);
  ~3DRenderer(void);

  long CreateDevice(const char *chAPI);
  LP3DRENDERDEVICE GetDevice(void) { return m_pDevice; }
  HINSTANCE GetModule(void) { return dll_; }
  void Release(void);

 private:
  3DRenderDevice * m_pDevice;
  HINSTANCE m_hInst;
  HMODULE dll_;
};

typedef 3DRenderer * LPSMT3DRENDERER;
}  // namespace _3Drd

#endif  //_RD3D_RENDERER_H