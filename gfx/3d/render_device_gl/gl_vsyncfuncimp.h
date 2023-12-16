#ifndef _VSYNC_FUNCSIMP_H
#define _VSYNC_FUNCSIMP_H

#include "gl_prerequisites.h"
#include "gl_vsyncfunc.h"

namespace _3Drd {
class VSyncFuncImpl : public VSyncFunc {
 public:
  VSyncFuncImpl();
  virtual ~VSyncFuncImpl();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual int WaitForVSync();
  virtual void EnableVSync();
  virtual void DisableVSync();

 private:
  PFNWGLSWAPINTERVALEXTPROC _wglSwapInterval;
};
}  // namespace _3Drd

#endif  //_VSYNC_FUNCSIMP_H
