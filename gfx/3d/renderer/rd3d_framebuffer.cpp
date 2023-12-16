#include "rd3d_framebuffer.h"

#include "rd3d_3drenderdevice.h"

namespace _3Drd {
FrameBuffer::FrameBuffer(LP3DRENDERDEVICE p3DRenderDevice, uint handle)
    : m_p3DRenderDevice(p3DRenderDevice), m_unHandle(handle) {}

FrameBuffer::~FrameBuffer() {}

long FrameBuffer::Use() { return m_p3DRenderDevice->BindFrameBuffer(this); }

long FrameBuffer::Unuse() { return m_p3DRenderDevice->UnbindFrameBuffer(); }

FrameBufferStatus FrameBuffer::CheckStatus() {
  return m_p3DRenderDevice->CheckFrameBufferStatus();
}

long FrameBuffer::AttachRenderBuffer(RenderBuffer *renderBuffer,
                                     RenderBufferSlot slot) {
  return m_p3DRenderDevice->AttachRenderBuffer(this, renderBuffer, slot);
}

long FrameBuffer::AttachTexture2D(Texture *texture2D, RenderBufferSlot slot) {
  return m_p3DRenderDevice->AttachTexture(this, texture2D, slot);
}
}  // namespace _3Drd