#ifndef _GLINDEXBUFFER_H
#define _GLINDEXBUFFER_H

#include "gl_prerequisites.h"
#include "rd3d_indexbuffer.h "


namespace _3Drd {
class GLIndexBuffer : public IndexBuffer {
 protected:
  GLIndexBuffer();
  GLIndexBuffer(const GLIndexBuffer&);
  GLIndexBuffer& operator=(const GLIndexBuffer&);

 public:
  GLIndexBuffer(int count);
  ~GLIndexBuffer();

  long PrepareForDrawing();
  long EndDrawing();

  long Lock();
  long Unlock();
  bool IsLocked() const { return m_bLocked; }

  void* GetIndexData();
  ulong GetIndexCount() const { return m_dwIndexCount; }
  ulong GetIndexStride() const { return m_dwStrideIndex; }

  void Index(uint index);

 private:
  bool m_bLocked;  // flag to specify if buffer is locked

  ulong m_dwIndexCount;   // number of vertex in buffer
  ulong m_dwStrideIndex;  // stride of entire index data
  uint* m_pIndex;         // pointer to head of current vertex

  //----------------------------------------------------------------------
  // OpenGL related members
  //--
  uint* m_pGLIndex;  // Buffer containing index data
};
}  // namespace _3Drd

#endif  //_GLINDEXBUFFER_H