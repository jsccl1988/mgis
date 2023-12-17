#ifndef _RD3D_SHADER_H
#define _RD3D_SHADER_H

#include "rd3d_3drenderdefs.h"

namespace _3Drd {
enum ShaderCompilationFlag {
  SCF_NOTHING = 0,
  SCF_CHECK_ERRORS = 1,
  SCF_LOG_ERRORS = SCF_CHECK_ERRORS | 2,
};

class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class CORE_EXPORT Shader {
 public:
  Shader(LP3DRENDERDEVICE p3DRenderDevice, uint handle, string strName);
  virtual ~Shader();

 public:
  inline uint GetHandle() { return m_unHandle; }
  const char* GetShaderName(void) { return m_strName.c_str(); }

 public:
  virtual long Load(string fileName, bool needToCompile = false,
                    ShaderCompilationFlag flags = SCF_NOTHING);
  virtual long Compile(ShaderCompilationFlag flags = SCF_LOG_ERRORS);
  virtual long IsCompiled();
  virtual char* GetCompileLog();

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  uint m_unHandle;
  string m_strName;
};
}  // namespace _3Drd
#endif  //_RD3D_SHADER_H