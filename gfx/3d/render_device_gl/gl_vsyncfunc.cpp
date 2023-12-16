#include "gl_vsyncfunc.h"

namespace _3Drd {
VSyncFunc::VSyncFunc() {}

VSyncFunc::~VSyncFunc() {}

long VSyncFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return SMT_ERR_NONE;
}

int VSyncFunc::WaitForVSync() { return 0; }

void VSyncFunc::EnableVSync() { ; }

void VSyncFunc::DisableVSync() { ; }
}  // namespace _3Drd