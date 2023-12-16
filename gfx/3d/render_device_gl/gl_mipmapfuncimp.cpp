#include "gl_mipmapfuncimp.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
MipmapFuncImpl::MipmapFuncImpl() {}

MipmapFuncImpl::~MipmapFuncImpl() {}

long MipmapFuncImpl::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  _glGenerateMipmap =
      (PFNGLGENERATEMIPMAPEXTPROC)pGLRenderDevice->GetProcAddress(
          "glGenerateMipmap");

  if (NULL == _glGenerateMipmap) {
    return SMT_ERR_FAILURE;
  }

  return SMT_ERR_NONE;
}

void MipmapFuncImpl::glGenerateMipmap(GLenum target) {
  _glGenerateMipmap(target);
}
}  // namespace _3Drd