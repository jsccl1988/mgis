#include "rd3d_shadermanager.h"

#include "rd3d_3drenderdevice.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
//////////////////////////////////////////////////////////////////////////
ShaderManager::ShaderManager(void) { DestroyAllShader(); }

ShaderManager::~ShaderManager(void) { DestroyAllShader(); }

long ShaderManager::AddShader(Shader *pShader) {
  Shader *pShaderTmp = NULL;
  pShaderTmp = GetShader(pShader->GetShaderName());
  if (NULL == pShaderTmp)
    m_mapNameToShaderPtrs.insert(
        pairNameToShaderPtr(pShader->GetShaderName(), pShader));
  else {
    LogManager *pLogMgr = LogManager::GetSingletonPtr();
    Log *pLog = pLogMgr->GetDefaultLog();

    pLog->LogMessage("AddShader () already exist");

    return ERR_FAILURE;
  }

  return ERR_NONE;
}

Shader *ShaderManager::GetShader(const char *name) {
  Shader *pShader = NULL;
  mapNameToShaderPtrs::iterator mapIter;
  mapIter = m_mapNameToShaderPtrs.find(name);

  if (mapIter != m_mapNameToShaderPtrs.end()) {
    pShader = (mapIter->second);
  }

  return pShader;
}

void ShaderManager::DestroyShader(const char *name) {
  mapNameToShaderPtrs::iterator iter = m_mapNameToShaderPtrs.find(name);

  if (iter != m_mapNameToShaderPtrs.end()) {
    SAFE_DELETE(iter->second);
    m_mapNameToShaderPtrs.erase(iter);
  }
}

void ShaderManager::DestroyAllShader(void) {
  mapNameToShaderPtrs::iterator i = m_mapNameToShaderPtrs.begin();

  while (i != m_mapNameToShaderPtrs.end()) {
    SAFE_DELETE(i->second);
    i++;
  }

  m_mapNameToShaderPtrs.clear();
}

void ShaderManager::GetAllShaderName(vector<string> &vStrAllShaderName) {
  vStrAllShaderName.clear();

  mapNameToShaderPtrs::iterator iter = m_mapNameToShaderPtrs.begin();

  while (iter != m_mapNameToShaderPtrs.end()) {
    vStrAllShaderName.push_back(iter->first);
    iter++;
  }
}
}  // namespace _3Drd