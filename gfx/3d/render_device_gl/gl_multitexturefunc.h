#ifndef _MULTITEXTURE_FUNCS_H
#define _MULTITEXTURE_FUNCS_H

#include "gl_prerequisites.h"

namespace _3Drd {
class GLRenderDevice;
typedef class GLRenderDevice *LPGLRENDERDEVICE;

class MultitextureFunc {
 public:
  MultitextureFunc();
  virtual ~MultitextureFunc();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glActiveTexture(GLenum texture);
};
}  // namespace _3Drd

#endif  //_MULTITEXTURE_FUNCS_H
