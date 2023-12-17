#include "rd3d_texturemanager.h"

#include "rd3d_3drenderdevice.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
TextureManager::TextureManager(void) { DestroyAllTexture(); }

TextureManager::~TextureManager(void) { DestroyAllTexture(); }

long TextureManager::AddTexture(Texture *pTexture) {
  if (NULL == pTexture) return ERR_FAILURE;

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetDefaultLog();

  Texture *pTexTmp = NULL;
  pTexTmp = GetTexture(pTexture->GetTextureName());
  if (NULL == pTexTmp)
    m_mapNameToTexturePtrs.insert(
        pairNameToTexturePtr(pTexture->GetTextureName(), pTexture));
  else {
    pLog->LogMessage("AddTexture () already exist");
    return ERR_FAILURE;
  }

  return ERR_NONE;
}

Texture *TextureManager::GetTexture(const char *name) {
  Texture *pTexture = NULL;
  mapNameToTexturePtrs::iterator mapIter;
  mapIter = m_mapNameToTexturePtrs.find(name);

  if (mapIter != m_mapNameToTexturePtrs.end()) {
    pTexture = (mapIter->second);
  }

  return pTexture;
}

long TextureManager::DestroyTexture(const char *name) {
  mapNameToTexturePtrs::iterator iter = m_mapNameToTexturePtrs.find(name);

  if (iter != m_mapNameToTexturePtrs.end()) {
    SAFE_DELETE(iter->second);
    m_mapNameToTexturePtrs.erase(iter);
  }

  return ERR_NONE;
}

long TextureManager::DestroyAllTexture(void) {
  mapNameToTexturePtrs::iterator i = m_mapNameToTexturePtrs.begin();

  while (i != m_mapNameToTexturePtrs.end()) {
    SAFE_DELETE(i->second);
    i++;
  }

  m_mapNameToTexturePtrs.clear();

  return ERR_NONE;
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