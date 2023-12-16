#include "gl_fbofunc.h"

namespace _3Drd {
FBOFunc::FBOFunc() {}

FBOFunc::~FBOFunc() {}

long FBOFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return SMT_ERR_NONE;
}

void FBOFunc::glGenFramebuffers(GLsizei count, GLuint *ids) { ; }

void FBOFunc::glDeleteFramebuffers(GLsizei count, GLuint *ids) { ; }

void FBOFunc::glBindFramebuffer(GLenum target, GLuint id) { ; }

GLboolean FBOFunc::glIsFramebuffer(GLuint id) { return false; }

void FBOFunc::glGenRenderbuffers(GLsizei count, GLuint *ids) { ; }

void FBOFunc::glDeleteRenderbuffers(GLsizei count, GLuint *ids) { ; }

void FBOFunc::glBindRenderbuffer(GLenum target, GLuint id) { ; }

GLboolean FBOFunc::glIsRenderbuffer(GLuint id) { return false; }

void FBOFunc::glRenderbufferStorage(GLenum target, GLenum internalFormat,
                                    GLsizei width, GLsizei height) {
  ;
}

void FBOFunc::glFramebufferRenderbuffer(GLenum target, GLenum attachment,
                                        GLenum rbTarget, GLuint rbId) {
  ;
}

int FBOFunc::getMaxColorAttachments() { return 0; }

void FBOFunc::glFramebufferTexture1D(GLenum target, GLenum attachment,
                                     GLenum texTarget, GLuint texId,
                                     int level) {
  ;
}

void FBOFunc::glFramebufferTexture2D(GLenum target, GLenum attachment,
                                     GLenum texTarget, GLuint texId,
                                     int level) {
  ;
}

void FBOFunc::glFramebufferTexture3D(GLenum target, GLenum attachment,
                                     GLenum texTarget, GLuint texId, int level,
                                     int zOffset) {
  ;
}

GLenum FBOFunc::glCheckFramebufferStatus(GLenum target) { return 0; }
}  // namespace _3Drd