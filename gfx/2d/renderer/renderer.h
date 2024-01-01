#ifndef GFX_2D_RENDERER_H
#define GFX_2D_RENDERER_H

#include <string>

#include "gfx/2d/renderer/render_export.h"
#include "gfx/2d/renderer/common.h"
#include "gfx/2d/renderer/render_device.h"


namespace gfx2d {
class GFX_2D_RENDERER_EXPORT Renderer {
 public:
  Renderer(HINSTANCE instance);
  ~Renderer(void);

  HRENDERDEVICE GetDevice(void) { return device_; }
  int CreateDevice(const char* device_name);
  void ReleaseDevice(void);

 private:
  std::string device_name_;
  HRENDERDEVICE device_;
  HINSTANCE instance_handle_;
  HMODULE dll_;
};

using HRENDERER = Renderer*;
}  // namespace gfx2d

#endif  // GFX_2D_RENDERER_H