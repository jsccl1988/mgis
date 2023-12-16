#ifndef _MULTITEXTURE_FUNCSIMP_H
#define _MULTITEXTURE_FUNCSIMP_H

#include "gl_multitexturefunc.h"
#include "gl_prerequisites.h"

namespace _3Drd {
class MultitextureFuncImpl : public MultitextureFunc {
 public:
  MultitextureFuncImpl();
  virtual ~MultitextureFuncImpl();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glActiveTexture(GLenum texture);

 private:
  PFNGLACTIVETEXTUREPROC _glActiveTexture;
};
}  // namespace _3Drd

#endif  //_MULTITEXTURE_FUNCSIMP_H
