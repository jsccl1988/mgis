#ifndef _RD3D_INDEXBUFFER_H
#define _RD3D_INDEXBUFFER_H
#include "rd3d_3drenderdefs.h"

namespace _3Drd {
class IndexBuffer {
 public:
  virtual ~IndexBuffer() {}

  virtual long Lock() = 0;
  virtual long Unlock() = 0;
  virtual bool IsLocked() const = 0;

  virtual void *GetIndexData() = 0;
  virtual ulong GetIndexCount() const = 0;

  virtual void Index(uint index) = 0;

  virtual long PrepareForDrawing() = 0;
  virtual long EndDrawing() = 0;
};
}  // namespace _3Drd

#endif  //_RD3D_INDEXBUFFER_H