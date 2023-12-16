#ifndef _GLVERTEXBUFFER_H
#define _GLVERTEXBUFFER_H

#include "gl_prerequisites.h"
#include "rd3d_vertexbuffer.h "


namespace _3Drd {
class GLVertexBuffer : public VertexBuffer {
 protected:
  GLVertexBuffer();
  GLVertexBuffer(const GLVertexBuffer &);
  GLVertexBuffer &operator=(const GLVertexBuffer &);

 public:
  GLVertexBuffer(int count, ulong format, bool isDynamic = true);
  ~GLVertexBuffer();

  long PrepareForDrawing();
  long EndDrawing();

  long Lock();
  long Unlock();
  bool IsLocked() const { return m_bLocked; }

  void *GetVertexData();

  ulong GetVertexCount() const { return m_dwVertexCount; }
  ulong GetVertexFormat() const { return m_dwFormat; }
  ulong GetVertexStride() const { return m_dwStrideVertex; }

  void Vertex(float x, float y, float z);
  void Vertex(float x, float y, float z, float w);
  void Normal(float x, float y, float z);
  void Diffuse(float r, float g, float b, float a = 1.0f);
  void TexVertex(float u, float v);

 private:
  bool m_bLocked;   // flag to specify if buffer is locked
  bool m_bDynamic;  // flag to specify if buffer is holding dynamic data

  ulong m_dwVertexCount;     // number of vertex in buffer
  ulong m_dwFormat;          // vertex format
  ulong m_dwStrideVertex;    // stride of entire vertex data
  ulong m_dwVertexCoordNum;  // Number of coordinates per vertex

  float *m_pVertex;    // pointer to head of current vertex
  float *m_pColor;     // pointer to head of current color
  float *m_pNormal;    // pointer to head of current normal
  float *m_pTexCoord;  // pointer to head of current texture coordinate

  //----------------------------------------------------------------------
  // OpenGL related members
  //--
  float *m_pGLVertices;   // Buffer containing vertex data
  float *m_pGLColors;     // Buffer containing vertex data
  float *m_pGLNormals;    // Buffer containing vertex data
  float *m_pGLTexCoords;  // Buffer containing vertex data
};
}  // namespace _3Drd

#endif  //_GLVERTEXBUFFER_H