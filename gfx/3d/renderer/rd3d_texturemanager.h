#ifndef _RD3D_TEXTUREMANAGER_H
#define _RD3D_TEXTUREMANAGER_H

#include <map>

#include "rd3d_texture.h"
#include "smt_filesys.h"


using namespace core;
using namespace _3Drd;

namespace _3Drd {
typedef vector<Texture*> vTexturePtrs;
typedef map<string, Texture*> mapNameToTexturePtrs;
typedef pair<string, Texture*> pairNameToTexturePtr;

class GIS_CORE_EXPORT TextureManager {
 public:
  TextureManager(void);
  virtual ~TextureManager(void);

 public:
  //	static Texture*				CreateTexture(LP3DRENDERDEVICE
  //pRenderDevice,FileInfo &info,const string strName);

 public:
  long AddTexture(Texture* pTexture);
  Texture* GetTexture(const char* szName);
  long DestroyTexture(const char* szName);
  long DestroyAllTexture(void);

  void GetAllTextureName(vector<string>& vStrAllTextureName);

 private:
  mapNameToTexturePtrs m_mapNameToTexturePtrs;
};
}  // namespace _3Drd

#endif  //_RD3D_TEXTUREMANAGER_H