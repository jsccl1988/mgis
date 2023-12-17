#include "gl_vbofuncimp.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
VBOFuncImpl::VBOFuncImpl() {}

VBOFuncImpl::~VBOFuncImpl() {}

long VBOFuncImpl::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  _glGenBuffers =
      (PFNGLGENBUFFERSPROC)pGLRenderDevice->GetProcAddress("glGenBuffersARB");
  _glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)pGLRenderDevice->GetProcAddress(
      "glDeleteBuffersARB");
  _glBindBuffer =
      (PFNGLBINDBUFFERPROC)pGLRenderDevice->GetProcAddress("glBindBufferARB");
  _glBufferData =
      (PFNGLBUFFERDATAPROC)pGLRenderDevice->GetProcAddress("glBufferDataARB");
  _glMapBuffer =
      (PFNGLMAPBUFFERPROC)pGLRenderDevice->GetProcAddress("glMapBufferARB");
  _glUnmapBuffer =
      (PFNGLUNMAPBUFFERPROC)pGLRenderDevice->GetProcAddress("glUnmapBufferARB");
  _glGetBufferParameteriv =
      (PFNGLGETBUFFERPARAMETERIVPROC)pGLRenderDevice->GetProcAddress(
          "glGetBufferParameterivARB");

  if (NULL == _glGenBuffers || NULL == _glDeleteBuffers ||
      NULL == _glBindBuffer || NULL == _glBufferData || NULL == _glMapBuffer ||
      NULL == _glUnmapBuffer || NULL == _glGetBufferParameteriv) {
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

void VBOFuncImpl::glGenBuffers(GLsizei count, GLuint *handle) {
  _glGenBuffers(count, handle);
}

void VBOFuncImpl::glDeleteBuffers(GLsizei count, const GLuint *handle) {
  _glDeleteBuffers(count, handle);
}

void VBOFuncImpl::glBindBuffer(GLenum target, GLuint handle) {
  _glBindBuffer(target, handle);
}

void VBOFuncImpl::glBufferData(GLenum target, GLsizeiptr size, GLvoid *data,
                               GLenum method) {
  _glBufferData(target, size, data, method);
}

void *VBOFuncImpl::glMapBuffer(GLenum target, GLenum access) {
  return _glMapBuffer(target, access);
}

GLboolean VBOFuncImpl::glUnmapBuffer(GLenum target) {
  return _glUnmapBuffer(target);
}

void VBOFuncImpl::glGetBufferParameteriv(GLenum target, GLenum param,
                                         GLint *value) {
  _glGetBufferParameteriv(target, param, value);
}

}  // namespace _3Drd