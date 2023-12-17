#include "gl_vbofunc.h"

namespace _3Drd {
VBOFunc::VBOFunc() {}

VBOFunc::~VBOFunc() {}

long VBOFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return ERR_NONE;
}

void VBOFunc::glGenBuffers(GLsizei count, GLuint *handle) { ; }

void VBOFunc::glDeleteBuffers(GLsizei count, const GLuint *handle) { ; }

void VBOFunc::glBindBuffer(GLenum target, GLuint handle) { ; }

void VBOFunc::glBufferData(GLenum target, GLsizeiptr size, GLvoid *data,
                           GLenum method) {
  ;
}

void *VBOFunc::glMapBuffer(GLenum target, GLenum access) { return NULL; }

GLboolean VBOFunc::glUnmapBuffer(GLenum target) { return GL_FALSE; }

void VBOFunc::glGetBufferParameteriv(GLenum target, GLenum param,
                                     GLint *value) {
  ;
}

}  // namespace _3Drd