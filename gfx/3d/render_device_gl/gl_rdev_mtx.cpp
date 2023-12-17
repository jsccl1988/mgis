#include "gl_3drenderdevice.h"
#include "smt_logmanager.h"

using namespace core;
namespace _3Drd {
//////////////////////////////////////////////////////////////////////////
// Tranformation functions
long GLRenderDevice::MatrixModeSet(MatrixMode mode) {
  m_matrixMode = mode;

  if (m_matrixMode == MM_PROJECTION)
    glMatrixMode(GL_PROJECTION);
  else
    glMatrixMode(GL_MODELVIEW);

  return ERR_NONE;
}

MatrixMode GLRenderDevice::MatrixModeGet() const { return m_matrixMode; }

long GLRenderDevice::MatrixLoadIdentity() {
  glLoadIdentity();
  return ERR_NONE;
}

long GLRenderDevice::MatrixLoad(const Matrix& mtx) {
  glLoadMatrixf((float*)(&mtx));
  return ERR_NONE;
}

long GLRenderDevice::MatrixPush() {
  glPushMatrix();
  return ERR_NONE;
}

long GLRenderDevice::MatrixPop() {
  glPopMatrix();
  return ERR_NONE;
}

long GLRenderDevice::MatrixScale(float x, float y, float z) {
  glScalef(x, y, z);
  return ERR_NONE;
}

long GLRenderDevice::MatrixTranslation(float x, float y, float z) {
  glTranslatef(x, y, z);
  return ERR_NONE;
}

long GLRenderDevice::MatrixRotation(float angle, float x, float y, float z) {
  glRotatef(angle, x, y, z);

  return ERR_NONE;
}

long GLRenderDevice::MatrixMultiply(const Matrix& mtx) {
  glMultMatrixf((float*)(&mtx));

  return ERR_NONE;
}

Matrix GLRenderDevice::MatrixGet() {
  Matrix mtxTmp;

  if (m_matrixMode == MM_PROJECTION)
    glGetFloatv(GL_PROJECTION_MATRIX, (float*)&mtxTmp);
  else
    glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&mtxTmp);

  return mtxTmp;
}
}  // namespace _3Drd