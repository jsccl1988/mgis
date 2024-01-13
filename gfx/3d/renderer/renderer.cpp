// Copyright (c) 2024 The mgis Authors.
// All rights reserved.

#include "gfx/3d/renderer/renderer.h"

#include "base/logging.h"
#include "base/util/string_util.h"

namespace gfx3d {
Renderer::Renderer(HINSTANCE instance) {
  instance_handle_ = instance;
  dll_ = nullptr;
  device_ = nullptr;
}

Renderer::~Renderer(void) { ReleaseDevice(); }

int Renderer::CreateDevice(const base::NameChar *device_name) {
  device_name_ = device_name;
  if (device_name_ == L"render_device_d3d") {
    dll_ = LoadLibrary(L"render_device_d3d.dll");
    if (!dll_) {
      std::string err = "   Loading ";
      err += base::UTF16ToUTF8(device_name_);
      err += " error!";
      LOG(ERROR) << __FUNCTION__ << err;
      return ERR_FAILURE;
    }

    CreateRenderDeviceFn create_render_device = 0;
    HRESULT hr;

    create_render_device =
        (CreateRenderDeviceFn)GetProcAddress(dll_, "CreateRenderDevice");

    if (nullptr == create_render_device) {
      return ERR_FAILURE;
    }

    hr = create_render_device(dll_, device_);

    if (FAILED(hr)) {
      device_ = nullptr;
      std::string err = "   Loading ";
      err += base::UTF16ToUTF8(device_name_);
      err += " error!";
      LOG(ERROR) << __FUNCTION__ << err;
      return ERR_FAILURE;
    }
  }

  return ERR_NONE;
}

void Renderer::ReleaseDevice(void) {
  DestroyRenderDeviceFn destroy_render_device = 0;

  if (dll_) {
    destroy_render_device =
        (DestroyRenderDeviceFn)GetProcAddress(dll_, "DestroyRenderDevice");
  } else {
    std::string err = "   Release ";
    err += base::UTF16ToUTF8(device_name_);
    err += " error!";
    LOG(ERROR) << __FUNCTION__ << err;
  }

  if (device_ && destroy_render_device) {
    destroy_render_device(device_);
  }
}
}  // namespace gfx3d