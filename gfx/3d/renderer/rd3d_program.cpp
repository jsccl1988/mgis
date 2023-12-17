#include "rd3d_program.h"

#include "rd3d_3drenderdevice.h"

namespace _3Drd {
Program::Program(LP3DRENDERDEVICE p3DRenderDevice, uint handle, string strName)
    : m_unHandle(handle),
      m_p3DRenderDevice(p3DRenderDevice),
      m_strName(strName) {
  ;
}

Program::~Program() { ; }

long Program::Use() { return m_p3DRenderDevice->BindProgram(this); }

long Program::Unuse() { return m_p3DRenderDevice->UnbindProgram(); }

long Program::SetVertexShader(Shader *shader) {
  return m_p3DRenderDevice->SetProgramVertexShader(this, shader);
}

long Program::SetPixelShader(Shader *shader) {
  return m_p3DRenderDevice->SetProgramPixelShader(this, shader);
}

long Program::Link(ShaderCompilationFlag flags) {
  if (ERR_NONE != m_p3DRenderDevice->LinkProgram(this))
    return ERR_FAILURE;

  /* Check if there is a need to check compilation */
  if (flags & SCF_CHECK_ERRORS) {
    long result = IsLinked();

    /* Check if there is a need to place errors in log file */
    if (ERR_NONE != result && (flags & SCF_LOG_ERRORS)) {
      ;
    }

    return result;
  }

  return ERR_NONE;
}

long Program::IsLinked() { return m_p3DRenderDevice->IsProgramLinked(this); }

char *Program::GetLinkLog() {
  return m_p3DRenderDevice->GetProgramLinkLog(this);
}

long Program::SetFloat(string param, float value) {
  return m_p3DRenderDevice->SetProgramFloat(this, param, value);
}

long Program::GetFloat(string param, float *value) {
  return m_p3DRenderDevice->GetProgramFloat(this, param, value);
}

long Program::SetVector(string param, const Vector2 &value) {
  return m_p3DRenderDevice->SetProgramVector(this, param, value);
}

long Program::SetVector(string param, const Vector3 &value) {
  return m_p3DRenderDevice->SetProgramVector(this, param, value);
}

long Program::SetVector(string param, const Vector4 &value) {
  return m_p3DRenderDevice->SetProgramVector(this, param, value);
}

long Program::SetTexture(string param, int texture) {
  return m_p3DRenderDevice->SetProgramTexture(this, param, texture);
}

long Program::SetInt(string param, int value) {
  return m_p3DRenderDevice->SetProgramInt(this, param, value);
}
}  // namespace _3Drd
