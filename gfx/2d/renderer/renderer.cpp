#include "rd_renderer.h"

namespace gfx2d {
Renderer::Renderer(HINSTANCE hInst) {
  m_hInst = hInst;
  dll_ = NULL;
  m_pDevice = NULL;
}

Renderer::~Renderer(void) { Release(); }

int Renderer::CreateDevice(const char *chAPI) {
  char buffer[300];

  if (strcmp(chAPI, "RenderDeviceGDI") == 0) {
    dll_ = LoadLibrary("RenderDeviceGDI.dll");
    if (!dll_) {
      ::MessageBox(NULL, "Loading RenderDeviceGDI.dll from dll failed.",
                   "SmartGis - error", MB_OK | MB_ICONERROR);
      return SMT_ERR_FAILURE;
    }

    _CreateRenderDevice _CreateRenderDev = 0;
    HRESULT hr;

    _CreateRenderDev =
        (_CreateRenderDevice)GetProcAddress(dll_, "CreateRenderDevice");

    if (NULL == _CreateRenderDev) return SMT_ERR_FAILURE;

    hr = _CreateRenderDev(dll_, m_pDevice);

    if (FAILED(hr)) {
      ::MessageBox(NULL, "CreateRenderDevice() from lib failed.", "Gis - error",
                   MB_OK | MB_ICONERROR);
      m_pDevice = NULL;

      return SMT_ERR_FAILURE;
    }

    return SMT_ERR_NONE;
  }

  void Renderer::Release(void) {
    _DestroyRenderDevice _ReleaseRenderDev = 0;

    if (dll_) {
      _ReleaseRenderDev =
          (_DestroyRenderDevice)GetProcAddress(dll_, "DestroyRenderDevice");
    }

    if (m_pDevice && _ReleaseRenderDev) {
      _ReleaseRenderDev(m_pDevice);
    }
  }
}  // namespace gfx2d