#ifndef _RD3D_PROGRAMMANAGER_H
#define _RD3D_PROGRAMMANAGER_H

#include <map>

#include "rd3d_program.h"


using namespace core;
using namespace _3Drd;

namespace _3Drd {
typedef vector<Program*> vProgramPtrs;
typedef map<string, Program*> mapNameToProgramPtrs;
typedef pair<string, Program*> pairNameToProgramPtr;

class GIS_CORE_EXPORT ProgramManager {
 public:
  ProgramManager(void);
  virtual ~ProgramManager(void);

 public:
  long AddProgram(Program* pProgram);
  Program* GetProgram(const char* szName);
  void DestroyProgram(const char* szName);
  void DestroyAllProgram(void);

  void GetAllProgramName(vector<string>& vStrAllProgramName);

 private:
  mapNameToProgramPtrs m_mapNameToProgramPtrs;
};
}  // namespace _3Drd

#endif  //_RD3D_PROGRAMMANAGER_H