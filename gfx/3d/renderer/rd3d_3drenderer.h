#ifndef _RD3D_RENDERER_H
#define _RD3D_RENDERER_H
#include "rd3d_3drenderdevice.h"

namespace _3Drd {
class CORE_EXPORT 3DRenderer {
 public:
  3DRenderer(HINSTANCE hInst);
  ~3DRenderer(void);

  long CreateDevice(const char *device_name);
  LP3DRENDERDEVICE GetDevice(void) { return device_; }
  HINSTANCE GetModule(void) { return dll_; }
  void Release(void);

 private:
  3DRenderDevice * device_;
  HINSTANCE instance_handle_;
  HMODULE dll_;
};

typedef 3DRenderer * LPSMT3DRENDERER;
}  // namespace _3Drd

#endif  //_RD3D_RENDERER_H