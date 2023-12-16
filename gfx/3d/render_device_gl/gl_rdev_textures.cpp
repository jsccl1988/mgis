#include "gl_3drenderdevice.h"
#include "smt_logmanager.h"
using namespace core;

namespace _3Drd {
Texture *GLRenderDevice::CreateTexture(const char *szName) {
  GLuint unHandle;
  glGenTextures(1, &unHandle);

  Texture *pTex = new Texture(this, unHandle, szName);
  if (SMT_ERR_NONE == m_textureMgr.AddTexture(pTex))
    return pTex;
  else {
    SMT_SAFE_DELETE(pTex);
    return NULL;
  }
}

long GLRenderDevice::GenerateMipmap(Texture *pTexture) {
  if (NULL == pTexture) return SMT_ERR_INVALID_PARAM;

  if (SMT_ERR_NONE == pTexture->Use()) {
    m_pFuncMipmap->glGenerateMipmap(GL_TEXTURE_2D);
    pTexture->Unuse();

    return SMT_ERR_NONE;
  }

  return SMT_ERR_FAILURE;
}

long GLRenderDevice::DestroyTexture(const char *szName) {
  Texture *pTexture = m_textureMgr.GetTexture(szName);

  if (NULL == pTexture) return SMT_ERR_INVALID_PARAM;

  GLhandleARB handle = pTexture->GetHandle();
  if (handle != 0) {
    glDeleteTextures(1, &handle);
  }

  m_textureMgr.DestroyTexture(pTexture->GetTextureName());

  return SMT_ERR_NONE;
}

Texture *GLRenderDevice::GetTexture(const char *szName) {
  return m_textureMgr.GetTexture(szName);
}

long GLRenderDevice::BindTexture(Texture *pTexture) {
  if (NULL == pTexture) return SMT_ERR_INVALID_PARAM;

  GLhandleARB handle = pTexture->GetHandle();
  glBindTexture(GL_TEXTURE_2D, handle);

  return SMT_ERR_NONE;
}

long GLRenderDevice::BuildTexture(Texture *pTexture) {
  if (NULL == pTexture) return SMT_ERR_INVALID_PARAM;

  void *pDataBuf = pTexture->GetData();

  if (NULL == pDataBuf) return SMT_ERR_INVALID_PARAM;

  TextureDesc texDesc = pTexture->GetDesc();

  GLuint internalFormat = ConvertTexFormat(texDesc.format);
  GLuint nativeFormat = 0;
  int components = pTexture->GetComponents(texDesc.format);

  GLenum target = GL_TEXTURE_2D;

  bool bPowerOfTwo = pTexture->IsSizePowerOfTwo();

  /*if (!bPowerOfTwo)
  target = GL_TEXTURE_RECTANGLE_ARB;*/

  if (components == 1)
    nativeFormat = GL_LUMINANCE;
  else {
    if (components == 3)
      nativeFormat = GL_RGB;
    else
      nativeFormat = GL_RGBA;
  }

  TextureSampler texSampler;
  TextureEnvMode texEvn;
  texSampler.rTexture = REPEAT;
  texSampler.sTexture = REPEAT;
  texSampler.tTexture = REPEAT;
  texEvn.envMode = MODULATE;

  //
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(target, pTexture->GetHandle());

  if (pTexture->IsUseMipmap()) {
    texSampler.magFilter = LINEAR_MIPMAP_LINEAR;
    texSampler.minFilter = LINEAR_MIPMAP_LINEAR;

    gluBuild2DMipmaps(target, components, texDesc.width, texDesc.height,
                      nativeFormat, GL_UNSIGNED_BYTE, pDataBuf);

    // m_p3DRenderDevice->GenerateMipmap(this);
  } else {
    texSampler.magFilter = LINEAR;
    texSampler.minFilter = LINEAR;

    glTexImage2D(target, 0, internalFormat, texDesc.width, texDesc.height, 0,
                 nativeFormat, GL_UNSIGNED_BYTE, pDataBuf);
  }

  pTexture->SetEnvMode(texEvn);
  pTexture->SetSampler(texSampler);

  return SMT_ERR_NONE;
}

long GLRenderDevice::UnbindTexture() {
  glBindTexture(GL_TEXTURE_2D, 0);

  return SMT_ERR_NONE;
}

long GLRenderDevice::BindRectTexture(Texture *pTexture) {
  if (NULL == pTexture) return SMT_ERR_INVALID_PARAM;

  GLhandleARB handle = pTexture->GetHandle();
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, handle);

  return SMT_ERR_NONE;
}

long GLRenderDevice::UnbindRectTexture() {
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

  return SMT_ERR_NONE;
}
}  // namespace _3Drd