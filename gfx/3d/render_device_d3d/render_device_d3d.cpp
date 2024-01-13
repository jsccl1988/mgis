
// Copyright (c) 2024 The Mogu Authors.
// All rights reserved.

#include "gfx/3d/render_device_d3d/render_device_d3d.h"

#include <math.h>

#include "base/logging.h"

#pragma warning(disable : 4344)

namespace gfx3d {
RenderDeviceD3D::RenderDeviceD3D(HINSTANCE instance) : RenderDevice(instance) {
  rhi_api_ = RHI3D_D3D;
}

RenderDeviceD3D::~RenderDeviceD3D(void) { Release(); }

int RenderDeviceD3D::Init(HWND hwnd) {
  if (hwnd == NULL) {
    LOG(ERROR) << __FUNCTION__ << "hwnd is nullptr!";
    return ERR_INVALID_PARAM;
  }

  hwnd_ = hwnd;
  LOG(INFO) << __FUNCTION__ << "Init D3D RenderDevice ok!";

  return ERR_NONE;
}

int RenderDeviceD3D::Destroy(void) {
  LOG(INFO) << __FUNCTION__ << "Destroy D3D RenderDevice ok!";
  return ERR_NONE;
}

int RenderDeviceD3D::Release(void) {
  LOG(INFO) << __FUNCTION__ << "Release D3D RenderDevice ok!";

  return ERR_NONE;
}
}  // namespace gfx3d