#ifndef _FBO_FUNCSIMP_H
#define _FBO_FUNCSIMP_H

#include "gl_fbofunc.h"
#include "gl_prerequisites.h"

namespace _3Drd {
class FBOFuncImpl : public FBOFunc {
 public:
  FBOFuncImpl();
  virtual ~FBOFuncImpl();

 public:
  virtual long Initialize(LPGLRENDERDEVICE pGLRenderDevice);

 public:
  virtual void glGenFramebuffers(GLsizei count, GLuint *ids);
  virtual void glDeleteFramebuffers(GLsizei count, GLuint *ids);
  virtual void glBindFramebuffer(GLenum target, GLuint id);
  virtual GLboolean glIsFramebuffer(GLuint id);
  virtual void glGenRenderbuffers(GLsizei count, GLuint *ids);
  virtual void glDeleteRenderbuffers(GLsizei count, GLuint *ids);
  virtual void glBindRenderbuffer(GLenum target, GLuint id);
  virtual GLboolean glIsRenderbuffer(GLuint id);
  virtual void glRenderbufferStorage(GLenum target, GLenum internalFormat,
                                     GLsizei width, GLsizei height);
  virtual void glFramebufferRenderbuffer(GLenum target, GLenum attachment,
                                         GLenum rbTarget, GLuint rbId);
  virtual int getMaxColorAttachments();
  virtual void glFramebufferTexture1D(GLenum target, GLenum attachment,
                                      GLenum texTarget, GLuint texId,
                                      int level);
  virtual void glFramebufferTexture2D(GLenum target, GLenum attachment,
                                      GLenum texTarget, GLuint texId,
                                      int level);
  virtual void glFramebufferTexture3D(GLenum target, GLenum attachment,
                                      GLenum texTarget, GLuint texId, int level,
                                      int zOffset);
  virtual GLenum glCheckFramebufferStatus(GLenum target);

 private:
  PFNGLGENFRAMEBUFFERSEXTPROC _glGenFramebuffers;
  PFNGLDELETEFRAMEBUFFERSEXTPROC _glDeleteFramebuffers;
  PFNGLBINDFRAMEBUFFEREXTPROC _glBindFramebuffer;
  PFNGLISFRAMEBUFFEREXTPROC _glIsFramebuffer;
  PFNGLGENRENDERBUFFERSEXTPROC _glGenRenderbuffers;
  PFNGLDELETERENDERBUFFERSEXTPROC _glDeleteRenderbuffers;
  PFNGLBINDRENDERBUFFEREXTPROC _glBindRenderbuffer;
  PFNGLISRENDERBUFFEREXTPROC _glIsRenderbuffer;
  PFNGLRENDERBUFFERSTORAGEEXTPROC _glRenderbufferStorage;
  PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC _glFramebufferRenderbuffer;
  PFNGLFRAMEBUFFERTEXTURE1DEXTPROC _glFramebufferTexture1D;
  PFNGLFRAMEBUFFERTEXTURE2DEXTPROC _glFramebufferTexture2D;
  PFNGLFRAMEBUFFERTEXTURE3DEXTPROC _glFramebufferTexture3D;
  PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC _glCheckFramebufferStatus;
};
}  // namespace _3Drd

#endif  //_FBO_FUNCSIMP_H
