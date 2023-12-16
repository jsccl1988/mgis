#ifndef _VSYNC_FUNCS_H
#define _VSYNC_FUNCS_H

#include "gl_prerequisites.h"

namespace _3Drd {
class GLRenderDevice;
typedef class GLRenderDevice *LPGLRENDERDEVICE;

class VSyncFunc {
 public:
  VSyncFunc();
  virtual ~VSyncFunc();
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual int WaitForVSync();
  virtual void EnableVSync();
  virtual void DisableVSync();
};
}  // namespace _3Drd

#endif  //_VSYNC_FUNCS_H
