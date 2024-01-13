// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_3D_RENDERER_RENDER_DEVICE_H
#define GFX_3D_RENDERER_RENDER_DEVICE_H

#include "gfx/3d/renderer/common.h"

namespace gfx3d {
class RenderDevice {
 public:
  RenderDevice(HINSTANCE instance)
      : rhi_api_(RHI3D_D3D), instance_handle_(instance), hwnd_(nullptr) {}

  virtual ~RenderDevice(void) {}

 public:
  virtual int Init(HWND hwnd) = 0;
  virtual int Destroy(void) = 0;
  virtual int Release(void) = 0;

 protected:
  HINSTANCE instance_handle_;
  RHI3D rhi_api_;

  HWND hwnd_;
  RenderOptions options_;
};
}  // namespace gfx3d

extern "C" {
using H3DRENDERDEVICE = gfx3d::RenderDevice *;
int GFX_3D_RENDERER_EXPORT CreateRenderDevice(HINSTANCE instance,
                                              H3DRENDERDEVICE &render_device);
int GFX_3D_RENDERER_EXPORT DestroyRenderDevice(H3DRENDERDEVICE &render_device);

using CreateRenderDeviceFn = HRESULT (*)(HINSTANCE instance,
                                         H3DRENDERDEVICE &render_device);
using DestroyRenderDeviceFn = HRESULT (*)(H3DRENDERDEVICE &render_device);
}

#endif  // GFX_3D_RENDERER_RENDER_DEVICE_H