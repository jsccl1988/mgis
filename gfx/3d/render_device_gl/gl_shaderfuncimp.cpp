#include "gl_shaderfuncimp.h"

#include "gl_3drenderdevice.h"

namespace _3Drd {
ShadersFuncImpl::ShadersFuncImpl() {}

ShadersFuncImpl::~ShadersFuncImpl() {}

long ShadersFuncImpl::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  _glCreateShader = (PFNGLCREATESHADERPROC)pGLRenderDevice->GetProcAddress(
      "glCreateShaderObjectARB");
  _glCreateProgram = (PFNGLCREATEPROGRAMPROC)pGLRenderDevice->GetProcAddress(
      "glCreateProgramObjectARB");
  _glAttachShader = (PFNGLATTACHSHADERPROC)pGLRenderDevice->GetProcAddress(
      "glAttachObjectARB");
  _glLinkProgram =
      (PFNGLLINKPROGRAMPROC)pGLRenderDevice->GetProcAddress("glLinkProgramARB");
  _glGetObjectParameteriv =
      (PFNGLGETOBJECTPARAMETERIVARBPROC)pGLRenderDevice->GetProcAddress(
          "glGetObjectParameterivARB");
  _glUseProgram = (PFNGLUSEPROGRAMPROC)pGLRenderDevice->GetProcAddress(
      "glUseProgramObjectARB");
  _glShaderSource = (PFNGLSHADERSOURCEARBPROC)pGLRenderDevice->GetProcAddress(
      "glShaderSourceARB");
  _glCompileShader = (PFNGLCOMPILESHADERARBPROC)pGLRenderDevice->GetProcAddress(
      "glCompileShaderARB");
  _glDetachShader = (PFNGLDETACHSHADERPROC)pGLRenderDevice->GetProcAddress(
      "glDetachObjectARB");
  _glDeleteObject = (PFNGLDELETEOBJECTARBPROC)pGLRenderDevice->GetProcAddress(
      "glDeleteObjectARB");
  _glGetInfoLog = (PFNGLGETINFOLOGARBPROC)pGLRenderDevice->GetProcAddress(
      "glGetInfoLogARB");
  _glGetUniformLocation =
      (PFNGLGETUNIFORMLOCATIONARBPROC)pGLRenderDevice->GetProcAddress(
          "glGetUniformLocationARB");
  _glUniform1fv = (PFNGLUNIFORM1FVARBPROC)pGLRenderDevice->GetProcAddress(
      "glUniform1fvARB");
  _glUniform2fv = (PFNGLUNIFORM2FVARBPROC)pGLRenderDevice->GetProcAddress(
      "glUniform2fvARB");
  _glUniform3fv = (PFNGLUNIFORM3FVARBPROC)pGLRenderDevice->GetProcAddress(
      "glUniform3fvARB");
  _glUniform4fv = (PFNGLUNIFORM4FVARBPROC)pGLRenderDevice->GetProcAddress(
      "glUniform4fvARB");
  _glUniform4f =
      (PFNGLUNIFORM4FARBPROC)pGLRenderDevice->GetProcAddress("glUniform4fARB");
  _glUniform1i =
      (PFNGLUNIFORM1IARBPROC)pGLRenderDevice->GetProcAddress("glUniform1iARB");
  _glGetUniformfv = (PFNGLGETUNIFORMFVARBPROC)pGLRenderDevice->GetProcAddress(
      "glGetUniformfvARB");
  _glGetUniformiv = (PFNGLGETUNIFORMIVARBPROC)pGLRenderDevice->GetProcAddress(
      "glGetUniformivARB");

  if (NULL == _glCreateShader || NULL == _glCreateProgram ||
      NULL == _glAttachShader || NULL == _glLinkProgram ||
      NULL == _glGetObjectParameteriv || NULL == _glUseProgram ||
      NULL == _glShaderSource || NULL == _glCompileShader ||
      NULL == _glDetachShader || NULL == _glDeleteObject ||
      NULL == _glGetInfoLog || NULL == _glGetUniformLocation ||
      NULL == _glUniform1fv || NULL == _glUniform2fv || NULL == _glUniform3fv ||
      NULL == _glUniform4fv || NULL == _glUniform1i ||
      NULL == _glGetUniformfv || NULL == _glGetUniformiv) {
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

GLuint ShadersFuncImpl::glCreateShader(GLenum type) {
  return _glCreateShader(type);
}

GLuint ShadersFuncImpl::glCreateProgram() { return _glCreateProgram(); }

void ShadersFuncImpl::glAttachShader(GLuint programId, GLuint shaderId) {
  _glAttachShader(programId, shaderId);
}

void ShadersFuncImpl::glLinkProgram(GLuint programId) {
  _glLinkProgram(programId);
}

void ShadersFuncImpl::glGetObjectParameteriv(GLhandleARB programId,
                                             GLenum target, GLint *value) {
  _glGetObjectParameteriv(programId, target, value);
}

void ShadersFuncImpl::glUseProgram(GLuint programId) {
  _glUseProgram(programId);
}

void ShadersFuncImpl::glShaderSource(
    GLuint shaderId, GLsizei size, const GLchar **source,
    const GLint *param)  // TODO: Fix definition
{
  _glShaderSource(shaderId, size, source, param);
}

void ShadersFuncImpl::glCompileShader(GLuint shaderId) {
  _glCompileShader(shaderId);
}

void ShadersFuncImpl::glDetachShader(GLuint programId, GLuint shaderId) {
  _glDetachShader(programId, shaderId);
}

void ShadersFuncImpl::glDeleteObject(
    GLhandleARB handle)  // TODO: Is it really about shaders?
{
  _glDeleteObject(handle);
}

void ShadersFuncImpl::glGetInfoLog(GLhandleARB programId, GLsizei size,
                                   GLsizei *written, GLcharARB *log) {
  _glGetInfoLog(programId, size, written, log);
}

GLint ShadersFuncImpl::glGetUniformLocation(GLuint shaderId,
                                            const GLchar *name) {
  return _glGetUniformLocation(shaderId, name);
}

void ShadersFuncImpl::glUniform1fv(GLint paramId, GLsizei size,
                                   const GLfloat *value) {
  _glUniform1fv(paramId, size, value);
}

void ShadersFuncImpl::glUniform2fv(GLint paramId, GLsizei size,
                                   const GLfloat *value) {
  _glUniform2fv(paramId, size, value);
}

void ShadersFuncImpl::glUniform3fv(GLint paramId, GLsizei size,
                                   const GLfloat *value) {
  _glUniform3fv(paramId, size, value);
}

void ShadersFuncImpl::glUniform4fv(GLint paramId, GLsizei size,
                                   const GLfloat *value) {
  _glUniform4fv(paramId, size, value);
}

void ShadersFuncImpl::glUniform4f(GLint paramId, float a, float b, float c,
                                  float d) {
  _glUniform4f(paramId, a, b, c, d);
}

void ShadersFuncImpl::glUniform1i(GLint paramId, GLint value) {
  _glUniform1i(paramId, value);
}

void ShadersFuncImpl::glGetUniformfv(GLuint shaderId, GLint paramId,
                                     GLfloat *values) {
  _glGetUniformfv(shaderId, paramId, values);
}

void ShadersFuncImpl::glGetUniformiv(GLuint shaderId, GLint paramId,
                                     GLint *values) {
  _glGetUniformiv(shaderId, paramId, values);
}

}  // namespace _3Drd