#include "gl_multitexturefunc.h"

namespace _3Drd {
MultitextureFunc::MultitextureFunc() {}

MultitextureFunc::~MultitextureFunc() {}

long MultitextureFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return SMT_ERR_NONE;
}

void MultitextureFunc::glActiveTexture(GLenum texture) { ; }

}  // namespace _3Drd