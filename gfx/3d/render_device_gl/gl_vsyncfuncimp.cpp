#include "gl_vsyncfuncimp.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
VSyncFuncImpl::VSyncFuncImpl() {}

VSyncFuncImpl::~VSyncFuncImpl() {}

long VSyncFuncImpl::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  _wglSwapInterval = (PFNWGLSWAPINTERVALEXTPROC)pGLRenderDevice->GetProcAddress(
      "wglSwapIntervalEXT");

  if (NULL == _wglSwapInterval) {
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

int VSyncFuncImpl::WaitForVSync() { return 0; }

void VSyncFuncImpl::EnableVSync() {
  _wglSwapInterval(1);
  ;
}

void VSyncFuncImpl::DisableVSync() { _wglSwapInterval(0); }
}  // namespace _3Drd