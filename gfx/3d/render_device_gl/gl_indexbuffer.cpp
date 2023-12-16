#include "gl_indexbuffer.h"

namespace _3Drd {
GLIndexBuffer::GLIndexBuffer(int count) : IndexBuffer() {
  ulong size = 0;

  m_dwIndexCount = count;

  m_bLocked = false;
  m_pIndex = NULL;
  m_pGLIndex = NULL;

  // Allocate memory for vertex data
  //--
  m_dwStrideIndex = sizeof(uint);
  m_pGLIndex = new uint[m_dwIndexCount];
}

GLIndexBuffer::~GLIndexBuffer() {
  if (m_pGLIndex != NULL) {
    delete[] m_pGLIndex;
    m_pGLIndex = NULL;
  }
}

long GLIndexBuffer::Lock() {
  m_bLocked = true;
  m_pIndex = m_pGLIndex;

  return SMT_ERR_NONE;
}

long GLIndexBuffer::Unlock() {
  m_bLocked = false;
  m_pIndex = NULL;

  return SMT_ERR_NONE;
}

void* GLIndexBuffer::GetIndexData() {
  if (NULL == m_pGLIndex) return NULL;

  return (void*)m_pGLIndex;
}

void GLIndexBuffer::Index(uint index) {
  *m_pIndex = index;
  m_pIndex++;
}

long GLIndexBuffer::PrepareForDrawing() {
  // Set pointers to arrays
  //--
  if (m_pGLIndex != NULL) {
    glEnableClientState(GL_INDEX_ARRAY);
    glIndexPointer(m_dwIndexCount, GL_UNSIGNED_INT, m_pGLIndex);
  } else {
    glDisableClientState(GL_INDEX_ARRAY);
  }

  return SMT_ERR_NONE;
}

long GLIndexBuffer::EndDrawing() {
  if (m_pGLIndex != NULL) {
    glDisableClientState(GL_INDEX_ARRAY);
  }

  return SMT_ERR_NONE;
}
}  // namespace _3Drd