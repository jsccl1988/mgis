#ifndef _RD3D_FRAMEBUFFER_H
#define _RD3D_FRAMEBUFFER_H

#include "rd3d_3drenderdefs.h"
#include "rd3d_renderbuffer.h"

namespace _3Drd {
enum RenderBufferSlot {
  COLOR_ATTACHMENT0 = 0,
  COLOR_ATTACHMENT1,
  COLOR_ATTACHMENT2,
  COLOR_ATTACHMENT3,
  COLOR_ATTACHMENT4,
  COLOR_ATTACHMENT5,
  COLOR_ATTACHMENT6,
  COLOR_ATTACHMENT7,
  DEPTH_ATTACHMENT,
  STENCIL_ATTACHMENT
};

enum FrameBufferStatus {
  FRAMEBUFFER_COMPLETE = 0,
  FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
  FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
  FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER,
  FRAMEBUFFER_INCOMPLETE_READ_BUFFER,
  FRAMEBUFFER_UNSUPPORTED
};

class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class GIS_CORE_EXPORT FrameBuffer {
 public:
  FrameBuffer(LP3DRENDERDEVICE p3DRenderDevice, uint handle);
  virtual ~FrameBuffer();

 public:
  inline uint GetHandle() { return m_unHandle; }

  long Use();
  long Unuse();

  FrameBufferStatus CheckStatus();
  long AttachRenderBuffer(RenderBuffer *renderBuffer, RenderBufferSlot slot);
  long AttachTexture2D(Texture *texture2D, RenderBufferSlot slot);

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  uint m_unHandle;
};
}  // namespace _3Drd

#endif  //_RD3D_FRAMEBUFFER_H