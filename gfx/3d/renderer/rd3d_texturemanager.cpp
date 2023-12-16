#include "rd3d_texturemanager.h"

#include "rd3d_3drenderdevice.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
TextureManager::TextureManager(void) { DestroyAllTexture(); }

TextureManager::~TextureManager(void) { DestroyAllTexture(); }

long TextureManager::AddTexture(Texture *pTexture) {
  if (NULL == pTexture) return SMT_ERR_FAILURE;

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetDefaultLog();

  Texture *pTexTmp = NULL;
  pTexTmp = GetTexture(pTexture->GetTextureName());
  if (NULL == pTexTmp)
    m_mapNameToTexturePtrs.insert(
        pairNameToTexturePtr(pTexture->GetTextureName(), pTexture));
  else {
    pLog->LogMessage("AddTexture () already exist");
    return SMT_ERR_FAILURE;
  }

  return SMT_ERR_NONE;
}

Texture *TextureManager::GetTexture(const char *szName) {
  Texture *pTexture = NULL;
  mapNameToTexturePtrs::iterator mapIter;
  mapIter = m_mapNameToTexturePtrs.find(szName);

  if (mapIter != m_mapNameToTexturePtrs.end()) {
    pTexture = (mapIter->second);
  }

  return pTexture;
}

long TextureManager::DestroyTexture(const char *szName) {
  mapNameToTexturePtrs::iterator iter = m_mapNameToTexturePtrs.find(szName);

  if (iter != m_mapNameToTexturePtrs.end()) {
    SMT_SAFE_DELETE(iter->second);
    m_mapNameToTexturePtrs.erase(iter);
  }

  return SMT_ERR_NONE;
}

long TextureManager::DestroyAllTexture(void) {
  mapNameToTexturePtrs::iterator i = m_mapNameToTexturePtrs.begin();

  while (i != m_mapNameToTexturePtrs.end()) {
    SMT_SAFE_DELETE(i->second);
    i++;
  }

  m_mapNameToTexturePtrs.clear();

  return SMT_ERR_NONE;
}

void TextureManager::GetAllTextureName(vector<string> &vStrAllTextureName) {
  vStrAllTextureName.clear();

  mapNameToTexturePtrs::iterator iter = m_mapNameToTexturePtrs.begin();

  while (iter != m_mapNameToTexturePtrs.end()) {
    vStrAllTextureName.push_back(iter->first);
    iter++;
  }
}
}  // namespace _3Drd