#include "gl_3drenderdevice.h"
#include "rd3d_programmanager.h"
#include "rd3d_shadermanager.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
// vertex shader
Shader *GLRenderDevice::CreateVertexShader(const char *name) {
  GLhandleARB newHandle = m_pFuncShaders->glCreateShader(GL_VERTEX_SHADER_ARB);
  Shader *pNewShader = new Shader(this, newHandle, name);

  if (ERR_NONE == m_shaderMgr.AddShader(pNewShader))
    return pNewShader;
  else {
    SAFE_DELETE(pNewShader);
    return NULL;
  }
}

// pixel shader
Shader *GLRenderDevice::CreatePixelShader(const char *name) {
  GLhandleARB newHandle =
      m_pFuncShaders->glCreateShader(GL_FRAGMENT_SHADER_ARB);
  Shader *pNewShader = new Shader(this, newHandle, name);

  if (ERR_NONE == m_shaderMgr.AddShader(pNewShader))
    return pNewShader;
  else {
    SAFE_DELETE(pNewShader);
    return NULL;
  }
}

Shader *GLRenderDevice::GetShader(const char *name) {
  return m_shaderMgr.GetShader(name);
}

// program
Program *GLRenderDevice::CreateProgram(const char *name) {
  GLhandleARB newHandle = m_pFuncShaders->glCreateProgram();
  Program *pNewProgram = new Program(this, newHandle, name);

  if (ERR_NONE == m_progamMgr.AddProgram(pNewProgram))
    return pNewProgram;
  else {
    SAFE_DELETE(pNewProgram);
    return NULL;
  }
}

Program *GLRenderDevice::GetProgram(const char *name) {
  return m_progamMgr.GetProgram(name);
}

long GLRenderDevice::LoadShaderSource(Shader *shader, char *source) {
  GLhandleARB handle = shader->GetHandle();
  m_pFuncShaders->glShaderSource(handle, 1, (const GLchar **)&source, NULL);

  return ERR_NONE;
}

long GLRenderDevice::CompileShader(Shader *shader) {
  GLhandleARB handle = shader->GetHandle();
  m_pFuncShaders->glCompileShader(handle);

  return ERR_NONE;
}

long GLRenderDevice::IsShaderCompiled(Shader *shader) {
  GLhandleARB handle = shader->GetHandle();
  int compileStatus = 0;

  m_pFuncShaders->glGetObjectParameteriv(handle, GL_OBJECT_COMPILE_STATUS_ARB,
                                         &compileStatus);
  if (compileStatus == 0) {
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

char *GLRenderDevice::GetShaderLog(Shader *shader) {
  if (NULL == shader) return NULL;

  GLchar *log = 0;
  int logLength = 0;
  int charsWritten = 0;
  GLhandleARB handle = shader->GetHandle();

  m_pFuncShaders->glGetObjectParameteriv(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                         &logLength);

  if (logLength < 1) return NULL;

  log = new GLchar[logLength];

  m_pFuncShaders->glGetInfoLog(handle, logLength, &charsWritten, log);

  return log;
}

long GLRenderDevice::DestroyShader(const char *name) {
  Shader *shader = m_shaderMgr.GetShader(name);
  if (NULL == shader) return ERR_FAILURE;

  GLhandleARB handle = shader->GetHandle();
  if (handle != 0) {
    m_pFuncShaders->glDeleteObject(handle);
  }

  m_shaderMgr.DestroyShader(shader->GetShaderName());

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
long GLRenderDevice::BindProgram(Program *program) {
  if (NULL == program) return ERR_FAILURE;

  m_pFuncShaders->glUseProgram(program->GetHandle());

  return ERR_NONE;
}

long GLRenderDevice::UnbindProgram() {
  m_pFuncShaders->glUseProgram(0);

  return ERR_NONE;
}

long GLRenderDevice::SetProgramVertexShader(Program *program, Shader *shader) {
  if (NULL == program || NULL == shader) return ERR_FAILURE;

  m_pFuncShaders->glAttachShader(program->GetHandle(), shader->GetHandle());

  return ERR_NONE;
}

long GLRenderDevice::SetProgramPixelShader(Program *program, Shader *shader) {
  if (NULL == program || NULL == shader) return ERR_FAILURE;

  m_pFuncShaders->glAttachShader(program->GetHandle(), shader->GetHandle());

  return ERR_NONE;
}

long GLRenderDevice::LinkProgram(Program *program) {
  if (NULL == program) return ERR_FAILURE;

  m_pFuncShaders->glLinkProgram(program->GetHandle());

  return ERR_NONE;
}

long GLRenderDevice::IsProgramLinked(Program *program) {
  if (NULL == program) return ERR_FAILURE;

  int linkStatus = 0;

  m_pFuncShaders->glGetObjectParameteriv(
      program->GetHandle(), GL_OBJECT_LINK_STATUS_ARB, &linkStatus);
  if (linkStatus == 0) {
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

char *GLRenderDevice::GetProgramLinkLog(Program *program) {
  if (NULL == program) return NULL;

  int logLength;
  char *log = NULL;
  GLhandleARB handle = program->GetHandle();
  m_pFuncShaders->glGetObjectParameteriv(handle, GL_OBJECT_INFO_LOG_LENGTH_ARB,
                                         &logLength);

  if (logLength != 1) {
    log = new GLchar[logLength];
    int charsWritten;

    m_pFuncShaders->glGetInfoLog(handle, logLength, &charsWritten, log);
  }

  return log;
}

long GLRenderDevice::DestroyProgram(const char *name) {
  Program *program = m_progamMgr.GetProgram(name);

  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  if (handle != 0) {
    m_pFuncShaders->glDeleteObject(handle);
  }

  m_progamMgr.DestroyProgram(program->GetProgramName());

  return ERR_NONE;
}

long GLRenderDevice::SetProgramVector(Program *program, string &param,
                                      const Vector4 &value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform4fv(loc, 1, (const GLfloat *)&value);

  return ERR_NONE;
}

long GLRenderDevice::SetProgramVector(Program *program, string &param,
                                      const Vector3 &value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform3fv(loc, 1, (const GLfloat *)&value);
  return ERR_NONE;
}

long GLRenderDevice::SetProgramVector(Program *program, string &param,
                                      const Vector2 &value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform2fv(loc, 1, (const GLfloat *)&value);

  return ERR_NONE;
}

long GLRenderDevice::SetProgramFloat(Program *program, string &param,
                                     float value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform1fv(loc, 1, (const GLfloat *)&value);

  return ERR_NONE;
}

long GLRenderDevice::SetProgramInt(Program *program, string &param, int value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform1i(loc, value);

  return ERR_NONE;
}

long GLRenderDevice::GetProgramFloat(Program *program, string &param,
                                     float *value) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glGetUniformfv(handle, loc, value);

  return ERR_NONE;
}

long GLRenderDevice::SetProgramTexture(Program *program, string &param,
                                       int texture) {
  if (NULL == program) return ERR_FAILURE;

  GLhandleARB handle = program->GetHandle();

  int loc = m_pFuncShaders->glGetUniformLocation(handle, param.c_str());
  if (loc < 0) return ERR_FAILURE;

  m_pFuncShaders->glUniform1i(loc, texture);

  return ERR_NONE;
}
}  // namespace _3Drd