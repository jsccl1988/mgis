#ifndef _MIPMAP_FUNCS_H
#define _MIPMAP_FUNCS_H

#include "gl_prerequisites.h"

namespace _3Drd {
class GLRenderDevice;
typedef class GLRenderDevice *LPGLRENDERDEVICE;

class MipmapFunc {
 public:
  MipmapFunc();
  virtual ~MipmapFunc();
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glGenerateMipmap(GLenum target);
};
}  // namespace _3Drd

#endif  //_MIPMAP_FUNCS_H
