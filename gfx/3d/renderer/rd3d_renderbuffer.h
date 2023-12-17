#ifndef _RD3D_RENDERBUFFER_H
#define _RD3D_RENDERBUFFER_H

#include "rd3d_base.h"
#include "rd3d_texture.h"

namespace _3Drd {
class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class CORE_EXPORT RenderBuffer {
 public:
  RenderBuffer(LP3DRENDERDEVICE p3DRenderDevice, uint handle,
               TextureFormat format, uint width, uint height)
      : m_p3DRenderDevice(p3DRenderDevice),
        m_unHandle(height),
        m_texFormat(format),
        m_unWidth(width),
        m_unHeight(height) {
    ;
  }

  virtual ~RenderBuffer() {}

 public:
  inline uint GetHandle(void) const { return m_unHandle; }

  inline uint GetWidth(void) const { return m_unWidth; }
  inline uint GetHeight(void) const { return m_unHeight; }
  inline TextureFormat GetFormat(void) const { return m_texFormat; }

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  uint m_unHandle;

  uint m_unWidth;
  uint m_unHeight;
  TextureFormat m_texFormat;
};
}  // namespace _3Drd

#endif  //_RD3D_RENDERBUFFER_H