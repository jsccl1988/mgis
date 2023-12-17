
#ifndef _RD3D_STATESMANAGER_H
#define _RD3D_STATESMANAGER_H

#include <stack>

#include "rd3d_3drenderdefs.h"
#include "rd3d_states.h"
#include "rd3d_texture.h"

namespace _3Drd {
/**
Defines different states of the pipeline that could
be set via StateManager class.
*/
enum PipelineState {
  /* Rasterizer states */
  ALPHA_TEST_STATE = 1 << 0,
  DEPTH_TEST_STATE = 1 << 1,
  BLEND_STATE = 1 << 2,
  COLOR_STATE = 1 << 3,

  /* Transform states */
  VIEWPORT_STATE = 1 << 4,
  MATRIX_STATE = 1 << 5,

  /* Group of the states */
  RASTERIZER_STATE =
      ALPHA_TEST_STATE | DEPTH_TEST_STATE | BLEND_STATE | COLOR_STATE,
  TRANSFORM_STATE = VIEWPORT_STATE | MATRIX_STATE,

  /* Pipeline state */
  GPU_STATE = RASTERIZER_STATE | TRANSFORM_STATE
};

/**
Manages all pipeline states.
*/
class CORE_EXPORT GPUStateManager {
 public:
  GPUState *GetState();
  virtual void PushStates(uint flags);
  virtual void PopStates(uint flags);

  // alpha test
  virtual AlphaTestState GetAlphaTestState(void) = 0;
  virtual long SetAlphaTestState(AlphaTestState &state) = 0;
  virtual long SetAlphaTest(bool enabled) = 0;
  virtual long SetAlphaTestFunc(Comparison func, float ref) = 0;

  // depth test
  virtual DepthTestState GetDepthTestState() = 0;
  virtual long SetDepthTestState(DepthTestState &state) = 0;
  virtual long SetDepthTest(bool enabled) = 0;
  virtual long SetDepthTestFunc(Comparison func) = 0;

  // blending
  virtual BlendState GetBlendState(void) = 0;
  virtual long SetBlendState(BlendState &state) = 0;
  virtual long SetBlending(bool enabled) = 0;

  // viewport
  virtual Viewport3D GetViewportState(void) = 0;
  virtual long SetViewportState(Viewport3D &vp) = 0;
  virtual long SetViewport(int x, int y, int width, int height);

  // clr
  virtual Color GetColorState(void) = 0;
  virtual long SetColorState(Color &colorState) = 0;
  virtual long SetColor(float red, float green, float blue, float alpha = 1.0);

  // texture
  virtual long Set2DTextures(bool enabled) = 0;
  virtual long Set2DRectTextures(bool enabled) = 0;
  virtual long SetSampler(TextureSampler &sampler) = 0;
  virtual long SetRectSampler(TextureSampler &sampler) = 0;
  virtual long SetTextureEnvironment(TextureEnvMode &envMode) = 0;

  // matrix
  virtual MatrixState GetMatrixState(void);
  virtual long SetMatrixState(MatrixState &state);
  virtual Matrix GetWorldViewMatrix(void) = 0;
  virtual Matrix GetProjectionMatrix(void) = 0;

  virtual long SetWorldViewMatrix(Matrix &matrix) = 0;
  virtual long SetProjectionMatrix(Matrix &matrix) = 0;

  // util
  virtual long GetClearColorValue(float &red, float &green, float &blue,
                                  float &alpha) = 0;
  virtual long SetClearColorValue(float red, float green, float blue,
                                  float alpha = 1.f) = 0;
  virtual long GetClearDepthValue(float &depth) = 0;
  virtual long SetClearDepthValue(float depth) = 0;
  virtual long GetStencilClearValue(ulong &s) = 0;
  virtual long SetStencilClearValue(ulong s) = 0;

  virtual long SetPolygonMode(FaceMode face, PolygonMode mode) = 0;

  virtual long GetLineWidth(float &size) = 0;
  virtual long SetLineWidth(float size) = 0;
  virtual long GetPointSize(float &size) = 0;
  virtual long SetPointSize(float size) = 0;

  virtual long EnableDepthOffset(PolygonMode mode, bool enabled) = 0;
  virtual long DepthOffsetParams(float rFactor, float dFactor) = 0;

  virtual long SetMaterail(bool enabled) = 0;
  virtual long SetLight(bool enabled) = 0;

 private:
  stack<AlphaTestState> alphaStack;
  stack<DepthTestState> depthStack;
  stack<BlendState> blendStack;
  stack<Viewport3D> viewportStack;
  stack<Color> colorStack;
  stack<MatrixState> matrixStack;
};
}  // namespace _3Drd

#endif  //_RD3D_STATESMANAGER_H