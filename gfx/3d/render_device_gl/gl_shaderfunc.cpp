#include "gl_shaderfunc.h"

namespace _3Drd {
ShadersFunc::ShadersFunc() {}

ShadersFunc::~ShadersFunc() {}

long ShadersFunc::Initialize(LPGLRENDERDEVICE pGLRenderDevice) {
  return SMT_ERR_NONE;
}

GLuint ShadersFunc::glCreateShader(GLenum type) { return 0; }

GLuint ShadersFunc::glCreateProgram() { return 0; }

void ShadersFunc::glAttachShader(GLuint programId, GLuint shaderId) { ; }

void ShadersFunc::glLinkProgram(GLuint programId) { ; }

void ShadersFunc::glGetObjectParameteriv(GLhandleARB programId, GLenum target,
                                         GLint *value) {
  ;
}

void ShadersFunc::glUseProgram(GLuint programId) { ; }

void ShadersFunc::glShaderSource(GLuint shaderId, GLsizei size,
                                 const GLchar **source,
                                 const GLint *param)  // TODO: Fix definition
{
  ;
}

void ShadersFunc::glCompileShader(GLuint shaderId) { ; }

void ShadersFunc::glDetachShader(GLuint programId, GLuint shaderId) { ; }

void ShadersFunc::glDeleteObject(
    GLhandleARB handle)  // TODO: Is it really about shaders?
{
  ;
}

void ShadersFunc::glGetInfoLog(GLhandleARB programId, GLsizei size,
                               GLsizei *written, GLcharARB *log) {
  ;
}

GLint ShadersFunc::glGetUniformLocation(GLuint shaderId, const GLchar *name) {
  return 0;
}

void ShadersFunc::glUniform1fv(GLint paramId, GLsizei size,
                               const GLfloat *value) {
  ;
}

void ShadersFunc::glUniform2fv(GLint paramId, GLsizei size,
                               const GLfloat *value) {
  ;
}

void ShadersFunc::glUniform3fv(GLint paramId, GLsizei size,
                               const GLfloat *value) {
  ;
}

void ShadersFunc::glUniform4fv(GLint paramId, GLsizei size,
                               const GLfloat *value) {
  ;
}

void ShadersFunc::glUniform4f(GLint paramId, float a, float b, float c,
                              float d) {
  ;
}

void ShadersFunc::glUniform1i(GLint paramId, GLint value) { ; }

void ShadersFunc::glGetUniformfv(GLuint shaderId, GLint paramId,
                                 GLfloat *values) {
  ;
}

void ShadersFunc::glGetUniformiv(GLuint shaderId, GLint paramId,
                                 GLint *values) {
  ;
}
}  // namespace _3Drd