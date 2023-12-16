#ifndef _RD3D_SHADERMANAGER_H
#define _RD3D_SHADERMANAGER_H

#include <map>

#include "rd3d_shader.h"
#include "smt_filesys.h"


using namespace core;
using namespace _3Drd;

namespace _3Drd {
typedef vector<Shader*> vShaderPtrs;
typedef map<string, Shader*> mapNameToShaderPtrs;
typedef pair<string, Shader*> pairNameToShaderPtr;

class GIS_CORE_EXPORT ShaderManager {
 public:
  ShaderManager(void);
  virtual ~ShaderManager(void);

 public:
  long AddShader(Shader* pShader);
  Shader* GetShader(const char* szName);
  void DestroyShader(const char* szName);
  void DestroyAllShader(void);

  void GetAllShaderName(vector<string>& vStrAllShaderName);

 private:
  mapNameToShaderPtrs m_mapNameToShaderPtrs;
};
}  // namespace _3Drd

#endif  //_RD3D_SHADERSMANAGER_H