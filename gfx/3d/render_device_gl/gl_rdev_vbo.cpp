#include "gl_3drenderdevice.h"
#include "smt_logmanager.h"

using namespace core;

namespace _3Drd {
// video buffer
VideoBuffer *GLRenderDevice::CreateVideoBuffer(ArrayType type) {
  GLhandleARB newHandle;
  m_pFuncVBO->glGenBuffers(1, &newHandle);
  VideoBuffer *newVideoBuffer = new VideoBuffer(this, newHandle, type);

  return newVideoBuffer;
}

long GLRenderDevice::BindBuffer(VideoBuffer *buffer) {
  GLhandleARB handle = buffer->GetHandle();
  m_pFuncVBO->glBindBuffer(GL_ARRAY_BUFFER, handle);

  return ERR_NONE;
}

long GLRenderDevice::BindIndexBuffer(VideoBuffer *buffer) {
  GLhandleARB handle = buffer->GetHandle();
  m_pFuncVBO->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);

  return ERR_NONE;
}

long GLRenderDevice::UnbindBuffer() {
  m_pFuncVBO->glBindBuffer(GL_ARRAY_BUFFER, 0);

  return ERR_NONE;
}

long GLRenderDevice::UnbindIndexBuffer() {
  m_pFuncVBO->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  return ERR_NONE;
}

long GLRenderDevice::UpdateBuffer(VideoBuffer *buffer, void *data, uint size,
                                  VideoBufferStoreMethod method) {
  if (NULL == buffer) return ERR_INVALID_PARAM;

  BindBuffer(buffer);
  GLenum GLMethod = ConvertVideoBufferStoreMethod(method);
  m_pFuncVBO->glBufferData(GL_ARRAY_BUFFER, size, data, GLMethod);

  return ERR_NONE;
}

long GLRenderDevice::UpdateIndexBuffer(VideoBuffer *buffer, void *data,
                                       uint size,
                                       VideoBufferStoreMethod method) {
  if (NULL == buffer || NULL == data) return ERR_INVALID_PARAM;

  BindIndexBuffer(buffer);

  GLenum GLMethod = ConvertVideoBufferStoreMethod(method);
  m_pFuncVBO->glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GLMethod);

  return ERR_NONE;
}

void *GLRenderDevice::MapBuffer(VideoBuffer *buffer, AccessMode access) {
  if (NULL == buffer) return NULL;

  GLhandleARB handle = buffer->GetHandle();
  GLenum glAccess = ConvertAccess(access);

  void *result = m_pFuncVBO->glMapBuffer(GL_ARRAY_BUFFER, glAccess);

  return result;
}

long GLRenderDevice::UnmapBuffer(VideoBuffer *buffer) {
  if (NULL == buffer) return ERR_INVALID_PARAM;

  GLboolean result = m_pFuncVBO->glUnmapBuffer(GL_ARRAY_BUFFER);

  return ERR_NONE;
}

void *GLRenderDevice::MapIndexBuffer(VideoBuffer *buffer, AccessMode access) {
  if (NULL == buffer) return NULL;

  GLhandleARB handle = buffer->GetHandle();
  GLenum glAccess = ConvertAccess(access);

  void *result = m_pFuncVBO->glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, glAccess);

  return result;
}

long GLRenderDevice::UnmapIndexBuffer(VideoBuffer *buffer) {
  if (NULL == buffer) return ERR_INVALID_PARAM;

  GLboolean result = m_pFuncVBO->glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

  return ERR_NONE;
}

long GLRenderDevice::DestroyBuffer(VideoBuffer *buffer) {
  GLhandleARB handle = buffer->GetHandle();
  m_pFuncVBO->glDeleteBuffers(1, &handle);

  return ERR_NONE;
}

long GLRenderDevice::DestroyIndexBuffer(VideoBuffer *buffer) {
  GLhandleARB handle = buffer->GetHandle();
  m_pFuncVBO->glDeleteBuffers(1, &handle);

  return ERR_NONE;
}
}  // namespace _3Drd