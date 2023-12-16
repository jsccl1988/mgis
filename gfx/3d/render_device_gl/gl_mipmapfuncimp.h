#ifndef _MIPMAP_FUNCSIMP_H
#define _MIPMAP_FUNCSIMP_H

#include "gl_mipmapfunc.h"
#include "gl_prerequisites.h"

namespace _3Drd {
class MipmapFuncImpl : public MipmapFunc {
 public:
  MipmapFuncImpl();
  virtual ~MipmapFuncImpl();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glGenerateMipmap(GLenum target);

 private:
  PFNGLGENERATEMIPMAPEXTPROC _glGenerateMipmap;
};
}  // namespace _3Drd

#endif  //_VSYNC_FUNCSIMP_H
