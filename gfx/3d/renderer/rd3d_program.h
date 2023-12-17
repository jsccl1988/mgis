#ifndef _RD3D_PROGRAM_H
#define _RD3D_PROGRAM_H

#include "rd3d_3drenderdefs.h"
#include "rd3d_base.h"
#include "rd3d_shader.h"

namespace _3Drd {
class 3DRenderDevice;
typedef class 3DRenderDevice * LP3DRENDERDEVICE;

class CORE_EXPORT Program {
 public:
  Program(LP3DRENDERDEVICE p3DRenderDevice, uint handle, string strName);
  virtual ~Program();

 public:
  inline uint GetHandle() { return m_unHandle; }
  const char *GetProgramName(void) { return m_strName.c_str(); }

 public:
  long Use();
  long Unuse();

  virtual long SetVertexShader(Shader *shader);
  virtual long SetPixelShader(Shader *shader);

  virtual long Link(ShaderCompilationFlag flags = SCF_LOG_ERRORS);
  virtual long IsLinked();
  virtual char *GetLinkLog();

  virtual long SetVector(string param, const Vector4 &value);
  virtual long SetVector(string param, const Vector3 &value);
  virtual long SetVector(string param, const Vector2 &value);
  virtual long SetFloat(string param, float value);
  virtual long SetInt(string param, int value);
  virtual long GetFloat(string param, float *value);
  virtual long SetTexture(string param, int texture);

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  uint m_unHandle;
  string m_strName;
};
}  // namespace _3Drd

#endif  //_RD3D_PROGRAM_H
