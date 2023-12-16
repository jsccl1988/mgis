#include "gl_3drenderdevice.h"
#include "gl_indexbuffer.h"
#include "gl_vertexbuffer.h"
#include "smt_logmanager.h"

using namespace core;

namespace _3Drd {
VertexBuffer* GLRenderDevice::CreateVertexBuffer(int nCount, DWORD dwFormat,
                                                 bool bDynamic) {
  GLVertexBuffer* pVB = new GLVertexBuffer(nCount, dwFormat, bDynamic);

  return pVB;
}

IndexBuffer* GLRenderDevice::CreateIndexBuffer(int nCount) {
  GLIndexBuffer* pIndex = new GLIndexBuffer(nCount);

  return pIndex;
}

// vba
long GLRenderDevice::SetVertexArray(int components, Type type, int stride,
                                    void* data) {
  glVertexPointer(components, ConvertType(type), stride, data);

  return SMT_ERR_NONE;
}

long GLRenderDevice::SetTextureCoordsArray(int components, Type type,
                                           int stride, void* data) {
  glTexCoordPointer(components, ConvertType(type), stride, data);

  return SMT_ERR_NONE;
}

long GLRenderDevice::SetNormalArray(Type type, int stride, void* data) {
  glNormalPointer(ConvertType(type), stride, data);

  return SMT_ERR_NONE;
}

long GLRenderDevice::SetIndexArray(Type type, int stride, void* data) {
  glIndexPointer(ConvertType(type), stride, data);

  return SMT_ERR_NONE;
}

long GLRenderDevice::EnableArray(ArrayType type, bool enabled) {
  if (enabled == true) {
    glEnableClientState(ConvertArrayType(type));
  } else {
    glDisableClientState(ConvertArrayType(type));
  }

  return SMT_ERR_NONE;
}
}  // namespace _3Drd