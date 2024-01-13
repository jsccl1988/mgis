// Copyright (c) 2024 The Mogu Authors.
// All rights reserved.

#ifndef GFX_3D_RENDER_DEVICE_RENDER_DEVICE_D3D_H
#define GFX_3D_RENDER_DEVICE_RENDER_DEVICE_D3D_H

#include <string>

#include "base/base_config.h"
#include "gfx/3d/renderer/render_device.h"

namespace gfx3d {
class RenderDeviceD3D : public RenderDevice {
 public:
  RenderDeviceD3D(HINSTANCE instance);
  virtual ~RenderDeviceD3D(void);

  int Init(HWND hwnd);
  int Destroy(void);
  int Release(void);
};
}  // namespace gfx3d

#endif  // GFX_3D_RENDER_DEVICE_RENDER_DEVICE_D3D_H