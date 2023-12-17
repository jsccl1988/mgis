#include "gl_3drenderdevice.h"
#include "gl_indexbuffer.h"
#include "gl_text.h"
#include "gl_vertexbuffer.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
//////////////////////////////////////////////////////////////////////////
// Rendering functions
long GLRenderDevice::BeginRender() {
  /*HDC hDC = ::GetDC(m_hWnd);

  wglMakeCurrent (hDC, m_hRC);

  ::ReleaseDC(m_hWnd,hDC);*/

  return ERR_NONE;
}

long GLRenderDevice::EndRender() {
  ::glFlush();
  // wglMakeCurrent(NULL,NULL);
  return ERR_NONE;
}

long GLRenderDevice::SwapBuffers() {
  HDC hDC = ::GetDC(m_hWnd);
  ::SwapBuffers(hDC);
  ::ReleaseDC(m_hWnd, hDC);

  return ERR_NONE;
}

long GLRenderDevice::DrawPrimitives(PrimitiveType primitiveType,
                                    VertexBuffer *pVB, DWORD baseVertex,
                                    DWORD primitiveCount) {
  if (pVB == NULL) return ERR_INVALID_PARAM;

  // Convert primitive type
  GLenum PT;
  ulong count;
  if (ERR_NONE !=
      GetOpenGLPrimitiveType(primitiveType, primitiveCount, &PT, &count))
    return ERR_FAILURE;

  // Say that the VB will be the source for our draw primitive calls
  //--
  if (ERR_NONE != pVB->PrepareForDrawing()) return ERR_FAILURE;

  // Draw primitives
  //--
  glDrawArrays(PT, baseVertex, count);

  if (ERR_NONE != pVB->EndDrawing()) return ERR_FAILURE;

  return ERR_NONE;
}

long GLRenderDevice::DrawIndexedPrimitives(PrimitiveType primitiveType,
                                           VertexBuffer *pVB, IndexBuffer *pIB,
                                           ulong baseIndex,
                                           ulong primitiveCount) {
  if (pVB == NULL || pIB == NULL) return ERR_INVALID_PARAM;

  // Convert primitive type
  GLenum PT;
  ulong count;
  if (ERR_NONE !=
      GetOpenGLPrimitiveType(primitiveType, primitiveCount, &PT, &count))
    return ERR_FAILURE;

  // Say that the VB will be the source for our draw primitive calls
  //--
  if (ERR_NONE != pVB->PrepareForDrawing() ||
      ERR_NONE != pIB->PrepareForDrawing())
    return ERR_FAILURE;

  // Draw primitives
  //--
  glDrawElements(PT, count, GL_UNSIGNED_INT, pIB->GetIndexData());

  if (ERR_NONE != pVB->EndDrawing() || ERR_NONE != pIB->EndDrawing())
    return ERR_FAILURE;

  return ERR_NONE;
}

inline long GLRenderDevice::GetOpenGLPrimitiveType(
    const PrimitiveType pt, const ulong nInitialPrimitiveCount,
    GLenum *GLPrimitiveType, ulong *nGLPrimitiveCount) {
  switch (pt) {
    case PT_POINTLIST:
      *GLPrimitiveType = GL_POINTS;
      *nGLPrimitiveCount = nInitialPrimitiveCount;
      return ERR_NONE;

    case PT_LINELIST:
      *GLPrimitiveType = GL_LINES;
      *nGLPrimitiveCount = 2 * nInitialPrimitiveCount;
      return ERR_NONE;

    case PT_LINESTRIP:
      *GLPrimitiveType = GL_LINE_STRIP;
      *nGLPrimitiveCount = nInitialPrimitiveCount;
      return ERR_NONE;

    case PT_TRIANGLELIST:
      *GLPrimitiveType = GL_TRIANGLES;
      *nGLPrimitiveCount = 3 * nInitialPrimitiveCount;
      return ERR_NONE;

    case PT_TRIANGLESTRIP:
      *GLPrimitiveType = GL_TRIANGLE_STRIP;
      *nGLPrimitiveCount = nInitialPrimitiveCount + 2;
      return ERR_NONE;

    case PT_TRIANGLEFAN:
      *GLPrimitiveType = GL_TRIANGLE_FAN;
      *nGLPrimitiveCount = nInitialPrimitiveCount + 2;
      return ERR_NONE;

    default:
      *GLPrimitiveType = GL_POINTS;
      *nGLPrimitiveCount = nInitialPrimitiveCount;
      return ERR_NONE;
  }

  return ERR_FAILURE;
}

long GLRenderDevice::DrawText(uint unID, float x, float y, float z,
                              const Color &color, const char *str, ...) {
  if (str == NULL || unID > m_vTextPtrs.size()) return ERR_INVALID_PARAM;

  GLText *pText = m_vTextPtrs.at(unID);
  if (NULL == pText) return ERR_INVALID_PARAM;

  char text[256];
  memset(text, '\0', 256);
  va_list args;

  va_start(args, str);
  vsprintf(text, str, args);
  va_end(args);

  glColor4f(color.fRed, color.fGreen, color.fBlue, color.fA);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  HDC hDC = ::GetDC(m_hWnd);
  pText->DrawText(hDC, x, y, z, text);
  ::ReleaseDC(m_hWnd, hDC);

  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  return ERR_NONE;
}

long GLRenderDevice::DrawText(uint unID, float x, float y, const Color &color,
                              const char *str, ...) {
  if (str == NULL || unID > m_vTextPtrs.size()) return ERR_INVALID_PARAM;

  GLText *pText = m_vTextPtrs.at(unID);
  if (NULL == pText) return ERR_INVALID_PARAM;

  char text[256];
  memset(text, '\0', 256);
  va_list args;

  va_start(args, str);
  vsprintf(text, str, args);
  va_end(args);

  glColor4f(color.fRed, color.fGreen, color.fBlue, color.fA);

  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);

  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, viewport[2], viewport[3], 0);
  glMatrixMode(GL_MODELVIEW);

  HDC hDC = ::GetDC(m_hWnd);
  pText->DrawText(hDC, x, y, text);
  ::ReleaseDC(m_hWnd, hDC);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  glEnable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);

  return ERR_NONE;
}
}  // namespace _3Drd