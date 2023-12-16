#ifndef _RD3D_STATES_H
#define _RD3D_STATES_H

#include "rd3d_3drenderdefs.h"
#include "rd3d_base.h"

namespace _3Drd {
struct AlphaTestState {
 public:
  AlphaTestState() : bEnabled(false), cmpFunc(CMP_GREATER), fRefValue(0) { ; }

  AlphaTestState(bool _bEnabled, Comparison _cmpFunc, float _fRefValue)
      : bEnabled(_bEnabled), cmpFunc(_cmpFunc), fRefValue(_fRefValue) {
    ;
  }

 public:
  bool bEnabled;
  Comparison cmpFunc;
  float fRefValue;
};

struct BlendState {
 public:
  BlendState() : bEnabled(false), srcFactor(BF_ONE), dstFactor(BF_ZERO) { ; }

  BlendState(bool _bEnabled, BlendFactor _srcFactor, BlendFactor _dstFactor)
      : bEnabled(_bEnabled), srcFactor(_srcFactor), dstFactor(_dstFactor) {
    ;
  }

 public:
  bool bEnabled;
  BlendFactor srcFactor;
  BlendFactor dstFactor;
};

struct DepthTestState {
 public:
  DepthTestState() : bEnabled(false), cmpFunc(CMP_GREATER), bDepthMask(false) {
    ;
  }

  DepthTestState(bool _bEnabled, Comparison _cmpFunc, bool _bDepthMask)
      : bEnabled(_bEnabled), cmpFunc(_cmpFunc), bDepthMask(_bDepthMask) {
    ;
  }

 public:
  bool bEnabled;
  Comparison cmpFunc;
  bool bDepthMask;
};

struct MatrixState {
 public:
  MatrixState() { ; }

  MatrixState(Matrix &_worldview, Matrix &_projection)
      : worldview(_worldview), projection(_projection) {
    ;
  }

 public:
  Matrix worldview;
  Matrix projection;
};

struct GPUState {
 public:
  Viewport3D viewport;
  BlendState blend;
  Color color;
  AlphaTestState alphaTest;
  DepthTestState depthTest;
  MatrixState wpmatrix;
};
}  // namespace _3Drd

#endif  //_RD3D_STATES_H