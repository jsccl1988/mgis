#ifndef _RD3D_VIDEOBUFFER_H
#define _RD3D_VIDEOBUFFER_H

#include "rd3d_3drenderdefs.h"

namespace _3Drd {
enum VideoBufferStoreMethod {
  STATIC_DRAW,
  STATIC_READ,
  STATIC_COPY,
  DYNAMIC_DRAW,
  DYNAMIC_READ,
  DYNAMIC_COPY,
  STREAM_DRAW,
  STREAM_READ,
  STREAM_COPY
};

enum ArrayType {
  TEXTURE_COORD_ARRAY, /*!< Texture coordinates array. */
  COLOR_ARRAY,         /*!< Color values array.        */
  INDEX_ARRAY,         /*!< Array of indices.          */
  NORMAL_ARRAY,        /*!< Array of normals.          */
  VERTEX_ARRAY         /*!< Vertex coordiantes array.  */
};

class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class GIS_CORE_EXPORT VideoBuffer {
 public:
  VideoBuffer(LP3DRENDERDEVICE p3DRenderDevice, uint handle, ArrayType type);
  virtual ~VideoBuffer();

 public:
  inline uint GetHandle() { return m_unHandle; }

  inline ArrayType GetArrayType() { return m_arType; }

  virtual long Use();
  virtual long Unuse();

  virtual long Update(void *data, unsigned int size,
                      VideoBufferStoreMethod method);

  virtual void *Map(AccessMode access);
  virtual long Unmap();

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  uint m_unHandle;

  ArrayType m_arType;
};
}  // namespace _3Drd

#endif  //_RD3D_VIDEOBUFFER_H