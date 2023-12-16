#ifndef _VBO_FUNCS_H
#define _VBO_FUNCS_H

#include "gl_prerequisites.h"

namespace _3Drd {
class GLRenderDevice;
typedef class GLRenderDevice *LPGLRENDERDEVICE;

class VBOFunc {
 public:
  VBOFunc();
  virtual ~VBOFunc();
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
};
}  // namespace _3Drd

#endif  //_VBO_FUNCS_H
