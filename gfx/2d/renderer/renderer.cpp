#include "gfx/2d/renderer/renderer.h"

#include "base/logging.h"

namespace gfx2d {
Renderer::Renderer(HINSTANCE instance) {
  instance_handle_ = instance;
  dll_ = nullptr;
  device_ = nullptr;
}

Renderer::~Renderer(void) { ReleaseDevice(); }

int Renderer::CreateDevice(const char *device_name) {
  device_name_ = device_name;
  if (strcmp(device_name, "render_device_gdi") == 0) {
    // dll_ = LoadLibrary("render_device_gdi.dll");
    if (!dll_) {
      std::string err = "   Loading ";
      err += device_name_;
      err += " error!";
      LOG(ERROR) << __FUNCTION__ << err;
      return ERR_FAILURE;
    }

    CreateRenderDeviceFn create_render_device = 0;
    HRESULT hr;

    create_render_device =
        (CreateRenderDeviceFn)GetProcAddress(dll_, "CreateRenderDevice");

    if (nullptr == create_render_device) return ERR_FAILURE;

    hr = create_render_device(dll_, device_);

    if (FAILED(hr)) {
      device_ = nullptr;
      std::string err = "   Loading ";
      err += device_name_;
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
    err += device_name_;
    err += " error!";
    LOG(ERROR) << __FUNCTION__ << err;
  }

  if (device_ && destroy_render_device) {
    destroy_render_device(device_);
  }
}
}  // namespace gfx2d