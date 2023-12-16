#ifndef _GL_STATESMANAGER_H
#define _GL_STATESMANAGER_H

#include <map>

#include "gl_prerequisites.h"
#include "rd3d_statesmanager.h"

namespace _3Drd {
class GLGPUStateManager : public GPUStateManager {
 public:
  GLGPUStateManager();

 public:
  // alpha test
  virtual AlphaTestState GetAlphaTestState(void);
  virtual long SetAlphaTestState(AlphaTestState &state);
  virtual long SetAlphaTest(bool enabled);
  virtual long SetAlphaTestFunc(Comparison func, float ref);

  // depth test
  virtual DepthTestState GetDepthTestState(void);
  virtual long SetDepthTestState(DepthTestState &state);
  virtual long SetDepthTest(bool enabled);
  virtual long SetDepthTestFunc(Comparison func);

  // blending
  virtual BlendState GetBlendState(void);
  virtual long SetBlendState(BlendState &state);
  virtual long SetBlending(bool enabled);

  // viewport
  virtual Viewport3D GetViewportState(void);
  virtual long SetViewportState(Viewport3D &vp);

  // clr
  virtual Color GetColorState(void);
  virtual long SetColorState(Color &colorState);

  // texture
  virtual long Set2DTextures(bool enabled);
  virtual long Set2DRectTextures(bool enabled);

  virtual long SetSampler(TextureSampler &sampler);
  virtual long SetRectSampler(TextureSampler &sampler);
  virtual long SetTextureEnvironment(TextureEnvMode &envMode);

  // matrix
  virtual Matrix GetWorldViewMatrix(void);
  virtual Matrix GetProjectionMatrix(void);

  virtual long SetWorldViewMatrix(Matrix &matrix);
  virtual long SetProjectionMatrix(Matrix &matrix);

  // util
  virtual long GetClearColorValue(float &red, float &green, float &blue,
                                  float &alpha);
  virtual long SetClearColorValue(float red, float green, float blue,
                                  float alpha = 1.f);
  virtual long GetClearDepthValue(float &depth);
  virtual long SetClearDepthValue(float depth);
  virtual long GetStencilClearValue(ulong &s);
  virtual long SetStencilClearValue(ulong s);

  virtual long SetPolygonMode(FaceMode face, PolygonMode mode);

  virtual long GetLineWidth(float &size);
  virtual long SetLineWidth(float size);
  virtual long GetPointSize(float &size);
  virtual long SetPointSize(float size);

  virtual long EnableDepthOffset(PolygonMode mode, bool enabled);
  virtual long DepthOffsetParams(float rFactor, float dFactor);

  virtual long SetMaterail(bool enabled);
  virtual long SetLight(bool enabled);

 private:
  int ConvertGLEnum(GLenum value);
  GLenum ConvertToGLEnum(uint tableIndex, uint value);

 private:
  map<int, int> m_GLToDev;
};
}  // namespace _3Drd

#endif  //_GL_STATESMANAGER_H