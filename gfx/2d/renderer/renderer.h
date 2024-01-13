// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#ifndef GFX_2D_RENDERER_H
#define GFX_2D_RENDERER_H

#include <string>

#include "base/file/file_path.h"
#include "gfx/2d/renderer/common.h"
#include "gfx/2d/renderer/render_device.h"
#include "gfx/2d/renderer/render_export.h"

namespace gfx2d {
class GFX_2D_RENDERER_EXPORT Renderer {
 public:
  Renderer(HINSTANCE instance);
  ~Renderer(void);

  H2DRENDERDEVICE GetDevice(void) { return device_; }
  int CreateDevice(const base::NameChar* device_name);
  void ReleaseDevice(void);

 private:
  base::NameString device_name_;
  H2DRENDERDEVICE device_;
  HINSTANCE instance_handle_;
  HMODULE dll_;
};
}  // namespace gfx2d

#endif  // GFX_2D_RENDERER_H