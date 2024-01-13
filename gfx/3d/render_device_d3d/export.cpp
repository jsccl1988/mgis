// Copyright (c) 2023 The Mogu Authors.
// All rights reserved.

#include "base/base_config.h"
#include "base/path/base_paths.h"
#include "gfx/3d/render_device_d3d/render_device_d3d.h"

extern "C" {
int CreateRenderDevice(HINSTANCE instance, H3DRENDERDEVICE &render_device) {
  if (!render_device) {
    render_device = new gfx3d::RenderDeviceD3D(instance);
    return ERR_NONE;
  }

  return ERR_FAILURE;
}

int DestroyRenderDevice(H3DRENDERDEVICE &render_device) {
  if (!render_device) {
    return ERR_FAILURE;
  }

  SAFE_DELETE(render_device);

  return ERR_NONE;
}
}

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason,
                               LPVOID reserved) {
  switch (reason) {
    case DLL_PROCESS_ATTACH: {
      base::PathString module_dir;
      base::PathProvider(base::FILE_MODULE_DIR, &module_dir);
      base::DirectoryAsDLLSearchPath(module_dir);
    }
      return TRUE;

    case DLL_PROCESS_DETACH:
      break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
  }

  return FALSE;
}
