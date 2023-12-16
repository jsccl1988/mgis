#include "rd3d_statesmanager.h"

namespace _3Drd {
GPUState* GPUStateManager::GetState() {
  GPUState* newState = new GPUState();
  newState->viewport = GetViewportState();
  newState->color = GetColorState();
  newState->blend = GetBlendState();
  newState->alphaTest = GetAlphaTestState();
  newState->depthTest = GetDepthTestState();
  return newState;
}

void GPUStateManager::PushStates(uint flags) {
  /* Rasterizer states */
  if (flags & ALPHA_TEST_STATE) {
    alphaStack.push(GetAlphaTestState());
  }

  if (flags & DEPTH_TEST_STATE) {
    depthStack.push(GetDepthTestState());
  }

  if (flags & BLEND_STATE) {
    blendStack.push(GetBlendState());
  }

  if (flags & COLOR_STATE) {
    colorStack.push(GetColorState());
  }

  /* Transform states */
  if (flags & VIEWPORT_STATE) {
    viewportStack.push(GetViewportState());
  }

  if (flags & MATRIX_STATE) {
    matrixStack.push(GetMatrixState());
  }
}

void GPUStateManager::PopStates(uint flags) {
  /* Rasterizer states */
  if (flags & ALPHA_TEST_STATE) {
    SetAlphaTestState(alphaStack.top());
    alphaStack.pop();
  }

  if (flags & DEPTH_TEST_STATE) {
    SetDepthTestState(depthStack.top());
    depthStack.pop();
  }

  if (flags & BLEND_STATE) {
    SetBlendState(blendStack.top());
    blendStack.pop();
  }

  if (flags & COLOR_STATE) {
    SetColorState(colorStack.top());
    colorStack.pop();
  }

  /* Transform states */
  if (flags & VIEWPORT_STATE) {
    SetViewportState(viewportStack.top());
    viewportStack.pop();
  }

  if (flags & MATRIX_STATE) {
    SetMatrixState(matrixStack.top());
    matrixStack.pop();
  }
}

long GPUStateManager::SetViewport(int x, int y, int width, int height) {
  return SetViewportState(Viewport3D(x, y, width, height, 0, 0, 0));
}

// clr
long GPUStateManager::SetColor(float red, float green, float blue,
                               float alpha) {
  return SetColorState(Color(red, green, blue, alpha));
}

// matrix
MatrixState GPUStateManager::GetMatrixState() {
  return MatrixState(GetWorldViewMatrix(), GetProjectionMatrix());
}

long GPUStateManager::SetMatrixState(MatrixState& state) {
  SetWorldViewMatrix(state.worldview);
  SetProjectionMatrix(state.projection);

  return SMT_ERR_NONE;
}
}  // namespace _3Drd
