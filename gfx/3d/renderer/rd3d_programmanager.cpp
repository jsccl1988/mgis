#include "rd3d_programmanager.h"

#include "rd3d_3drenderdevice.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
//////////////////////////////////////////////////////////////////////////
ProgramManager::ProgramManager(void) { DestroyAllProgram(); }

ProgramManager::~ProgramManager(void) { DestroyAllProgram(); }

long ProgramManager::AddProgram(Program *pProgram) {
  Program *pProgTmp = NULL;
  pProgTmp = GetProgram(pProgram->GetProgramName());
  if (NULL == pProgTmp)
    m_mapNameToProgramPtrs.insert(
        pairNameToProgramPtr(pProgram->GetProgramName(), pProgram));
  else {
    LogManager *pLogMgr = LogManager::GetSingletonPtr();
    Log *pLog = pLogMgr->GetDefaultLog();

    pLog->LogMessage("AddProgram () already exist");

    return ERR_FAILURE;
  }

  return ERR_NONE;
}

Program *ProgramManager::GetProgram(const char *name) {
  Program *pProgram = NULL;
  mapNameToProgramPtrs::iterator mapIter;
  mapIter = m_mapNameToProgramPtrs.find(name);

  if (mapIter != m_mapNameToProgramPtrs.end()) {
    pProgram = (mapIter->second);
  }

  return pProgram;
}

void ProgramManager::DestroyProgram(const char *name) {
  mapNameToProgramPtrs::iterator iter = m_mapNameToProgramPtrs.find(name);

  if (iter != m_mapNameToProgramPtrs.end()) {
    SAFE_DELETE(iter->second);
    m_mapNameToProgramPtrs.erase(iter);
  }
}

void ProgramManager::DestroyAllProgram(void) {
  mapNameToProgramPtrs::iterator i = m_mapNameToProgramPtrs.begin();

  while (i != m_mapNameToProgramPtrs.end()) {
    SAFE_DELETE(i->second);
    i++;
  }

  m_mapNameToProgramPtrs.clear();
}

void ProgramManager::GetAllProgramName(vector<string> &vStrAllProgramName) {
  vStrAllProgramName.clear();

  mapNameToProgramPtrs::iterator iter = m_mapNameToProgramPtrs.begin();

  while (iter != m_mapNameToProgramPtrs.end()) {
    vStrAllProgramName.push_back(iter->first);
    iter++;
  }
}
}  // namespace _3Drd