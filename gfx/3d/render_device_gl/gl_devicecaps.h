#ifndef _GL_3DDEVICECAPS_H
#define _GL_3DDEVICECAPS_H

#include "rd3d_3ddevicecaps.h"

namespace _3Drd {
class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class GLDeviceCaps : public 3DDeviceCaps {
 public:
  GLDeviceCaps(LP3DRENDERDEVICE p3DRenderDevice);
  virtual ~GLDeviceCaps(void){};

 public:
  virtual bool IsVBOSupported();
  virtual bool IsMipMapsSupported();
  virtual bool IsFBOSupported();
  virtual bool IsGLSLSupported();
  virtual bool IsVSyncSupported();
  virtual bool IsMultiTextureSupported();
  virtual bool IsAnisotropySupported();
  virtual int GetTextureSlotsCount();
  virtual int GetMaxColorAttachments();
  virtual float GetMaxAnisotropy();

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
};
}  // namespace _3Drd

#endif  //_GL_3DDEVICECAPS_H
