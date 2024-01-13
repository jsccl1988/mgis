// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_3D_RENDERER_H
#define GFX_3D_RENDERER_H

#include <string>

#include "base/file/file_path.h"
#include "gfx/3d/renderer/common.h"
#include "gfx/3d/renderer/render_device.h"
#include "gfx/3d/renderer/render_export.h"

namespace gfx3d {
class GFX_3D_RENDERER_EXPORT Renderer {
 public:
  Renderer(HINSTANCE instance);
  ~Renderer(void);

  H3DRENDERDEVICE GetDevice(void) { return device_; }
  int CreateDevice(const base::NameChar* device_name);
  void ReleaseDevice(void);

 private:
  base::NameString device_name_;
  H3DRENDERDEVICE device_;
  HINSTANCE instance_handle_;
  HMODULE dll_;
};

}  // namespace gfx3d

#endif  // GFX_3D_RENDERER_H