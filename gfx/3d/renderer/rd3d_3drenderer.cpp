#include "rd3d_3drenderer.h"

namespace _3Drd {
3DRenderer ::3DRenderer(HINSTANCE hInst) {
  instance_handle_ = hInst;
  device_ = NULL;
  dll_ = NULL;
}

3DRenderer ::~3DRenderer(void) { Release(); }

long 3DRenderer ::CreateDevice(const char *device_name) {
  char buffer[300];

  if (strcmp(device_name, "OpenGL") == 0) {
#ifdef _DEBUG
    dll_ = LoadLibrary("GLRenderDeviceD.dll");
    if (!dll_) {
      ::MessageBox(NULL, "Loading GLRenderDeviceD.dll from lib failed.",
                   "SmartGis - error", MB_OK | MB_ICONERROR);
      return FALSE;
    }
#else
    dll_ = LoadLibrary("GLRenderDevice.dll");
    if (!dll_) {
      ::MessageBox(NULL, "Loading GLRenderDevice.dll from lib failed.",
                   "SmartGis - error", MB_OK | MB_ICONERROR);
      return FALSE;
    }
#endif
  } else if (strcmp(device_name, "Direct3D") == 0) {
#ifdef _DEBUG
    dll_ = LoadLibrary("D3DRenderDeviceD.dll");
    if (!dll_) {
      ::MessageBox(NULL, "Loading D3DRenderDeviceD.dll from lib failed.",
                   "SmartGis - error", MB_OK | MB_ICONERROR);
      return FALSE;
    }
#else
    dll_ = LoadLibrary("D3DRenderDevice.dll");
    if (!dll_) {
      ::MessageBox(NULL, "Loading D3DRenderDevice.dll from lib failed.",
                   "SmartGis - error", MB_OK | MB_ICONERROR);
      return FALSE;
    }
#endif
  } else {
    _snprintf(buffer, 300, "API '%s' not yet supported.", device_name);
    ::MessageBox(NULL, buffer, "SmartGis - error", MB_OK | MB_ICONERROR);
    return FALSE;
  }

  _Create3DRenderDevice _CreateRDev = 0;
  HRESULT hr;

  _CreateRDev =
      (_Create3DRenderDevice)GetProcAddress(dll_, "Create3DRenderDevice");

  hr = _CreateRDev(dll_, device_);

  if (FAILED(hr)) {
    ::MessageBox(NULL, "Create3DRenderDevice() from lib failed.",
                 "SmartGis - error", MB_OK | MB_ICONERROR);
    device_ = NULL;

    return ERR_FAILURE;
  }

  return ERR_NONE;
}

void 3DRenderer ::Release(void) {
  _Release3DRenderDevice _ReleaseRDev = 0;
  HRESULT hr;

  if (dll_) {
    _ReleaseRDev =
        (_Release3DRenderDevice)GetProcAddress(dll_, "Release3DRenderDevice");
  }

  if (device_) {
    hr = _ReleaseRDev(device_);
    if (FAILED(hr)) {
      device_ = NULL;
    }
  }
}
}  // namespace _3Drd