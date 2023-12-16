#include "gl_multitexturefuncimp.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
MultitextureFuncImpl::MultitextureFuncImpl() {}

MultitextureFuncImpl::~MultitextureFuncImpl() {}

long MultitextureFuncImpl::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  _glActiveTexture = (PFNGLACTIVETEXTUREPROC)pGLRenderDevice->GetProcAddress(
      "glActiveTextureARB");

  if (NULL == _glActiveTexture) {
    return SMT_ERR_FAILURE;
  }

  return SMT_ERR_NONE;
}

void MultitextureFuncImpl::glActiveTexture(GLenum texture) {
  _glActiveTexture(texture);
}
}  // namespace _3Drd