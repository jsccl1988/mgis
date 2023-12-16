#ifndef _VBO_FUNCSIMP_H
#define _VBO_FUNCSIMP_H

#include "gl_prerequisites.h"
#include "gl_vbofunc.h"

namespace _3Drd {
class VBOFuncImpl : public VBOFunc {
 public:
  VBOFuncImpl();
  virtual ~VBOFuncImpl();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glGenBuffers(GLsizei count, GLuint *handle);
  virtual void glDeleteBuffers(GLsizei count, const GLuint *handle);
  virtual void glBindBuffer(GLenum target, GLuint handle);
  virtual void glBufferData(GLenum target, GLsizeiptr size, GLvoid *data,
                            GLenum method);
  virtual void *glMapBuffer(GLenum target, GLenum access);
  virtual GLboolean glUnmapBuffer(GLenum target);
  virtual void glGetBufferParameteriv(GLenum target, GLenum param,
                                      GLint *value);

 private:
  PFNGLGENBUFFERSPROC _glGenBuffers;
  PFNGLDELETEBUFFERSPROC _glDeleteBuffers;
  PFNGLBINDBUFFERPROC _glBindBuffer;
  PFNGLBUFFERDATAPROC _glBufferData;
  PFNGLMAPBUFFERPROC _glMapBuffer;
  PFNGLUNMAPBUFFERPROC _glUnmapBuffer;
  PFNGLGETBUFFERPARAMETERIVPROC _glGetBufferParameteriv;
};
}  // namespace _3Drd

#endif  //_VBO_FUNCSIMP_H
