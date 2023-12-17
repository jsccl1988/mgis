#include "gl_3drenderdevice.h"
#include "smt_logmanager.h"
using namespace core;

namespace _3Drd {
FrameBuffer *GLRenderDevice::CreateFrameBuffer() {
  GLhandleARB handle;
  m_pFuncFBO->glGenFramebuffers(1, &handle);
  FrameBuffer *newFB = new FrameBuffer(this, handle);

  return newFB;
}

long GLRenderDevice::DestroyFrameBuffer(FrameBuffer *frameBuffer) {
  if (frameBuffer) {
    GLhandleARB handle = frameBuffer->GetHandle();
    m_pFuncFBO->glDeleteFramebuffers(1, &handle);
  }

  return ERR_NONE;
}

long GLRenderDevice::BindFrameBuffer(FrameBuffer *frameBuffer) {
  if (frameBuffer) {
    GLhandleARB handle = frameBuffer->GetHandle();
    m_pFuncFBO->glBindFramebuffer(GL_FRAMEBUFFER_EXT, handle);
  }

  return ERR_NONE;
}

long GLRenderDevice::UnbindFrameBuffer() {
  m_pFuncFBO->glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
  return ERR_NONE;
}

RenderBuffer *GLRenderDevice::CreateRenderBuffer(TextureFormat format,
                                                 uint width, uint height) {
  GLhandleARB handle;
  GLenum rbFormat = ConvertTexFormat(format);

  m_pFuncFBO->glGenRenderbuffers(1, &handle);
  m_pFuncFBO->glBindRenderbuffer(GL_RENDERBUFFER_EXT, handle);
  m_pFuncFBO->glRenderbufferStorage(GL_RENDERBUFFER_EXT, rbFormat, width,
                                    height);

  RenderBuffer *pRenderBuf =
      new RenderBuffer(this, handle, format, width, height);

  return pRenderBuf;
}

long GLRenderDevice::DestroyRenderBuffer(RenderBuffer *renderBuffer) {
  GLhandleARB handle = renderBuffer->GetHandle();
  m_pFuncFBO->glDeleteRenderbuffers(1, &handle);
  return ERR_NONE;
}

long GLRenderDevice::AttachRenderBuffer(FrameBuffer *frameBuffer,
                                        RenderBuffer *renderBuffer,
                                        RenderBufferSlot slot) {
  GLhandleARB frmHandle = frameBuffer->GetHandle();
  GLhandleARB rdhandle = renderBuffer->GetHandle();

  GLenum rbSlot = ConvertRenderBufferSlot(slot);
  m_pFuncFBO->glFramebufferRenderbuffer(GL_FRAMEBUFFER_EXT, rbSlot,
                                        GL_RENDERBUFFER_EXT, rdhandle);

  return ERR_NONE;
}

long GLRenderDevice::AttachTexture(FrameBuffer *frameBuffer, Texture *texture2D,
                                   RenderBufferSlot slot) {
  GLenum rbSlot = ConvertRenderBufferSlot(slot);
  GLhandleARB id = texture2D->GetHandle();
  m_pFuncFBO->glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, rbSlot, GL_TEXTURE_2D,
                                     id, 0);
  return ERR_NONE;
}

FrameBufferStatus GLRenderDevice::CheckFrameBufferStatus() {
  GLenum result = m_pFuncFBO->glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
  switch (result) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      return FRAMEBUFFER_COMPLETE;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      return FRAMEBUFFER_INCOMPLETE_ATTACHMENT;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      return FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      return FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      return FRAMEBUFFER_INCOMPLETE_READ_BUFFER;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      return FRAMEBUFFER_UNSUPPORTED;
    default:
      return FRAMEBUFFER_UNSUPPORTED;
  }
}

GLenum GLRenderDevice::ConvertRenderBufferSlot(RenderBufferSlot slot) {
  GLenum map[] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT,
                  GL_COLOR_ATTACHMENT2_EXT, GL_COLOR_ATTACHMENT3_EXT,
                  GL_COLOR_ATTACHMENT4_EXT, GL_COLOR_ATTACHMENT5_EXT,
                  GL_COLOR_ATTACHMENT6_EXT, GL_COLOR_ATTACHMENT7_EXT,
                  GL_DEPTH_ATTACHMENT_EXT,  GL_STENCIL_ATTACHMENT_EXT};

  int index = static_cast<int>(slot);
  if (index > 0 && index <= sizeof(map) / sizeof(GLenum))
    return map[index];
  else
    return -1;
}
}  // namespace _3Drd