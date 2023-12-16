#ifndef _RD3D_VTERTEXBUFFER_H
#define _RD3D_VTERTEXBUFFER_H
#include "rd3d_3drenderdefs.h"

namespace _3Drd {
class VertexBuffer {
 public:
  virtual ~VertexBuffer() {}

  virtual long Lock() = 0;
  virtual long Unlock() = 0;

  virtual bool IsLocked() const = 0;
  virtual void *GetVertexData() = 0;
  virtual ulong GetVertexCount() const = 0;
  virtual ulong GetVertexFormat() const = 0;
  virtual ulong GetVertexStride() const = 0;

  virtual void Vertex(float x, float y, float z) = 0;
  virtual void Vertex(float x, float y, float z, float w) = 0;

  virtual void Normal(float x, float y, float z) = 0;
  virtual void Diffuse(float r, float g, float b, float a = 1.0f) = 0;
  virtual void TexVertex(float u, float v) = 0;

  virtual long PrepareForDrawing() = 0;
  virtual long EndDrawing() = 0;
};
}  // namespace _3Drd

#endif  //_RD3D_VTERTEXBUFFER_H