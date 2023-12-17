#include "gl_mipmapfunc.h"

namespace _3Drd {
MipmapFunc::MipmapFunc() {}

MipmapFunc::~MipmapFunc() {}

long MipmapFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return ERR_NONE;
}

void MipmapFunc::glGenerateMipmap(GLenum target) { ; }
}  // namespace _3Drd