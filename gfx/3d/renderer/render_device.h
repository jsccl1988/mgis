// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_3D_RENDERER_RENDER_DEVICE_H
#define GFX_3D_RENDERER_RENDER_DEVICE_H

#include "gfx/3d/renderer/common.h"

namespace gfx3d {
class RenderDevice {
 public:
  RenderDevice(HINSTANCE instance)
      : rhi_api_(RHI3D_FLYCUBE), instance_handle_(instance), hwnd_(nullptr) {}

  virtual ~RenderDevice(void) {}

 public:
  virtual int Init(HWND hwnd) {
    hwnd_ = hwnd;
    return 0;
  }
  virtual int Destroy(void) { return 0; }
  virtual int Release(void) { return 0; }

  // Same shape as sg::rhi Encoder + Submit. D3D shell is frozen.
  void Begin(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f) {
    (void)r;
    (void)g;
    (void)b;
    (void)a;
    rhi_closed_ = false;
  }
  void SetPipeline(const char *name) { (void)name; }
  void SetMesh(const float *xyz, unsigned vertex_count, const unsigned *idx,
               unsigned index_count) {
    (void)xyz;
    vertex_count_ = vertex_count;
    (void)idx;
    index_count_ = index_count;
  }
  void Draw(unsigned vertex_count) {
    (void)vertex_count;
    ++rhi_draw_count_;
  }
  void DrawIndexed(unsigned index_count) {
    (void)index_count;
    ++rhi_draw_indexed_count_;
  }
  void End() { rhi_closed_ = true; }
  bool closed() const { return rhi_closed_; }
  void Submit() {
    if (rhi_closed_) {
      ++rhi_submit_count_;
    }
  }
  unsigned long long submit_count() const { return rhi_submit_count_; }
  unsigned long long draw_count() const { return rhi_draw_count_; }

 protected:
  HINSTANCE instance_handle_;
  RHI3D rhi_api_;

  HWND hwnd_;
  RenderOptions options_;

  bool rhi_closed_ = true;
  unsigned vertex_count_ = 0;
  unsigned index_count_ = 0;
  unsigned long long rhi_submit_count_ = 0;
  unsigned long long rhi_draw_count_ = 0;
  unsigned long long rhi_draw_indexed_count_ = 0;
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