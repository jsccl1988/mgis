#include "rd3d_videobuffer.h"

#include "rd3d_3drenderdevice.h"

namespace _3Drd {
VideoBuffer::VideoBuffer(LP3DRENDERDEVICE p3DRenderDevice, uint handle,
                         ArrayType type)
    : m_p3DRenderDevice(p3DRenderDevice), m_unHandle(handle), m_arType(type) {}

VideoBuffer::~VideoBuffer() { ; }

long VideoBuffer::Use() {
  if (m_arType == INDEX_ARRAY) {
    m_p3DRenderDevice->BindIndexBuffer(this);
  } else {
    m_p3DRenderDevice->BindBuffer(this);
  }

  return ERR_NONE;
}

long VideoBuffer::Unuse() {
  if (m_arType == INDEX_ARRAY) {
    m_p3DRenderDevice->UnbindIndexBuffer();
  } else {
    m_p3DRenderDevice->UnbindBuffer();
  }

  return ERR_NONE;
}

void* VideoBuffer::Map(AccessMode access) {
  if (m_arType == INDEX_ARRAY) {
    return m_p3DRenderDevice->MapIndexBuffer(this, access);
  } else {
    return m_p3DRenderDevice->MapBuffer(this, access);
  }
}

long VideoBuffer::Unmap() {
  if (m_arType == INDEX_ARRAY) {
    m_p3DRenderDevice->UnmapIndexBuffer(this);
  } else {
    m_p3DRenderDevice->UnmapBuffer(this);
  }
  return ERR_NONE;
}

long VideoBuffer::Update(void* data, unsigned int size,
                         VideoBufferStoreMethod method) {
  if (m_arType == INDEX_ARRAY) {
    m_p3DRenderDevice->UpdateIndexBuffer(this, data, size, method);
  } else {
    m_p3DRenderDevice->UpdateBuffer(this, data, size, method);
  }
  return ERR_NONE;
}
}  // namespace _3Drd