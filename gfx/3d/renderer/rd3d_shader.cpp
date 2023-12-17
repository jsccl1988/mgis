#include "rd3d_shader.h"

#include "rd3d_3drenderdevice.h"

namespace _3Drd {
Shader::Shader(LP3DRENDERDEVICE p3DRenderDevice, uint handle, string strName)
    : m_unHandle(handle),
      m_p3DRenderDevice(p3DRenderDevice),
      m_strName(strName) {
  ;
}

Shader::~Shader() { ; }

long Shader::Load(std::string fileName, bool needToCompile,
                  ShaderCompilationFlag flags) {
  std::vector<char> data;

  FILE* sFile = fopen(fileName.c_str(), "rb");
  if (NULL == sFile) {
    return ERR_INVALID_FILE;
  }

  char buf[1024];
  while (!feof(sFile)) {
    int readBytes = fread(buf, 1, sizeof(buf), sFile);
    for (int i = 0; i < readBytes; i++) {
      data.push_back(buf[i]);
    }
  }
  fclose(sFile);

  data.push_back(0);  // To get NULL-terminated string from vector

  if (ERR_NONE !=
      m_p3DRenderDevice->LoadShaderSource(this, (char*)&data[0]))
    return ERR_FAILURE;

  if (needToCompile) {
    return Compile(flags);
  }

  return ERR_NONE;
}

long Shader::Compile(ShaderCompilationFlag flags) {
  if (ERR_NONE != m_p3DRenderDevice->CompileShader(this))
    return ERR_FAILURE;

  /* Check if there is a need to check compilation */
  if (flags & SCF_CHECK_ERRORS) {
    long result = IsCompiled();

    /* Check if there is a need to place errors in log file */
    if (result != ERR_NONE && (flags & SCF_LOG_ERRORS)) {
      GetCompileLog();
    }

    return result;
  }

  return ERR_NONE;
}

long Shader::IsCompiled() { return m_p3DRenderDevice->IsShaderCompiled(this); }

char* Shader::GetCompileLog() { return m_p3DRenderDevice->GetShaderLog(this); }

}  // namespace _3Drd
