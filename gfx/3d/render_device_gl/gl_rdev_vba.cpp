#include "gl_3drenderdevice.h"
#include "gl_indexbuffer.h"
#include "gl_vertexbuffer.h"
#include "smt_logmanager.h"

using namespace core;

namespace _3Drd {
VertexBuffer* GLRenderDevice::CreateVertexBuffer(int size, DWORD dwFormat,
                                                 bool bDynamic) {
  GLVertexBuffer* pVB = new GLVertexBuffer(size, dwFormat, bDynamic);

  return pVB;
}

IndexBuffer* GLRenderDevice::CreateIndexBuffer(int size) {
  GLIndexBuffer* pIndex = new GLIndexBuffer(size);

  return pIndex;
}

// vba
long GLRenderDevice::SetVertexArray(int components, Type type, int stride,
                                    void* data) {
  glVertexPointer(components, ConvertType(type), stride, data);

  return ERR_NONE;
}

long GLRenderDevice::SetTextureCoordsArray(int components, Type type,
                                           int stride, void* data) {
  glTexCoordPointer(components, ConvertType(type), stride, data);

  return ERR_NONE;
}

long GLRenderDevice::SetNormalArray(Type type, int stride, void* data) {
  glNormalPointer(ConvertType(type), stride, data);

  return ERR_NONE;
}

long GLRenderDevice::SetIndexArray(Type type, int stride, void* data) {
  glIndexPointer(ConvertType(type), stride, data);

  return ERR_NONE;
}

long GLRenderDevice::EnableArray(ArrayType type, bool enabled) {
  if (enabled == true) {
    glEnableClientState(ConvertArrayType(type));
  } else {
    glDisableClientState(ConvertArrayType(type));
  }

  return ERR_NONE;
}
}  // namespace _3Drd