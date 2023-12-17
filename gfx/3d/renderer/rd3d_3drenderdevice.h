#ifndef _RD3D_3DRENDERDEVICE_H
#define _RD3D_3DRENDERDEVICE_H

#include <vector>

#include "rd3d_3ddevicecaps.h"
#include "rd3d_base.h"
#include "rd3d_framebuffer.h"
#include "rd3d_indexbuffer.h"
#include "rd3d_programmanager.h"
#include "rd3d_shadermanager.h"
#include "rd3d_statesmanager.h"
#include "rd3d_texturemanager.h"
#include "rd3d_vertexbuffer.h"
#include "rd3d_videobuffer.h"


namespace _3Drd {
class 3DRenderDevice {
 public:
  3DRenderDevice(void)
      : dll_(NULL),
        m_rBaseApi(RA_OPENGL),
        m_strLogName("default"),
        m_bBlending(true){};
  virtual ~3DRenderDevice(void){};

 public:
  virtual long Init(HWND hWnd, const char *logname) = 0;
  virtual long Destroy(void) = 0;
  virtual long Release(void) = 0;

  inline RenderBase3DApi GetBaseApi() const { return m_rBaseApi; }

 public:
  virtual GPUStateManager *GetStateManager() = 0;
  virtual 3DDeviceCaps * GetDeviceCaps() = 0;

 public:
  // render
  virtual long BeginRender() = 0;
  virtual long EndRender() = 0;

  virtual long DrawPrimitives(PrimitiveType type, VertexBuffer * pVB,
                              ulong baseVertex, ulong primitiveCount) = 0;
  virtual long DrawIndexedPrimitives(PrimitiveType type, VertexBuffer * pVB,
                                     IndexBuffer * pIB, ulong baseIndex,
                                     ulong primitiveCount) = 0;

  virtual long DrawText(uint unID, float xpos, float ypos, float zpos,
                        const Color &color, const char *, ...) = 0;
  virtual long DrawText(uint unID, float xscreen, float yscreen,
                        const Color &color, const char *, ...) = 0;

  virtual long SwapBuffers() = 0;

  virtual long SetClearColor(const Color &clr) = 0;
  virtual long SetDepthClearValue(float z) = 0;
  virtual long SetStencilClearValue(ulong s) = 0;
  virtual long Clear(ulong flags) = 0;

  // matrix
  virtual long MatrixModeSet(MatrixMode mode) = 0;
  virtual MatrixMode MatrixModeGet() const = 0;
  virtual long MatrixLoadIdentity() = 0;

  virtual long MatrixLoad(const Matrix &m) = 0;
  virtual long MatrixPush() = 0;
  virtual long MatrixPop() = 0;
  virtual long MatrixScale(float x, float y, float z) = 0;
  virtual long MatrixTranslation(float x, float y, float z) = 0;
  virtual long MatrixRotation(float angle, float x, float y, float z) = 0;
  virtual long MatrixMultiply(const Matrix &m) = 0;
  virtual Matrix MatrixGet() = 0;

  // get frustum
  virtual long GetFrustum(Frustum & frustum) = 0;

  // view
  virtual long SetViewport(Viewport3D & viewport) = 0;
  virtual Viewport3D &GetViewport(void) = 0;

  virtual long SetOrtho(float left, float right, float bottom, float top,
                        float zNear, float zFar) = 0;
  virtual long SetPerspective(float fovy, float aspect, float zNear,
                              float zFar) = 0;

  // set viewlookat
  virtual long SetViewLookAt(Vector3 & vPos, Vector3 & vView,
                             Vector3 & vUp) = 0;

  // 2d to 3d
  virtual long Transform2DTo3D(Vector3 & vOrg, Vector3 & vTar,
                               const lPoint &point) = 0;

  // 3d to 2d
  virtual long Transform3DTo2D(const Vector3 &ver3D, lPoint &point) = 0;

 public:
  // aactivate additive blending
  virtual long SetBlending(bool bBlending) = 0;

  // activate backface culling
  virtual long SetBackfaceCulling(RenderStateValue rsv) = 0;

  // activate stencil buffer
  virtual long SetStencilBufferMode(RenderStateValue rsv, ulong ul) = 0;

  // activate depth buffer
  virtual long SetDepthBufferMode(RenderStateValue rsv) = 0;

  // activate wireframe mode
  virtual long SetShadeMode(RenderStateValue rsv, float v,
                            const Color &clr) = 0;

 public:
  // light stuff
  virtual long SetLight(int index, Light *pLight) = 0;
  virtual long SetAmbientLight(const Color &clr) = 0;

  // set texture
  virtual long SetTexture(Texture * pTex) = 0;

  // material stuff
  virtual long SetMaterial(Material * pMat) = 0;

  // fog stuff
  virtual long SetFog(FogMode mode, const Color &colour, float density,
                      float start, float end) = 0;

 public:
  //////////////////////////////////////////////////////////////////////////vbuf
  // vertex buffer
  virtual VertexBuffer *CreateVertexBuffer(int nCount, ulong unFormat,
                                           bool bDynamic = true) = 0;

  // index buffer
  virtual IndexBuffer *CreateIndexBuffer(int nCount) = 0;

  // video buffer
  virtual VideoBuffer *CreateVideoBuffer(ArrayType type) = 0;
  virtual long DestroyBuffer(VideoBuffer * buffer) = 0;
  virtual long DestroyIndexBuffer(VideoBuffer * buffer) = 0;

  // vba
  virtual long SetVertexArray(int components, Type type, int stride,
                              void *data) = 0;
  virtual long SetTextureCoordsArray(int components, Type type, int stride,
                                     void *data) = 0;
  virtual long SetNormalArray(Type type, int stride, void *data) = 0;
  virtual long SetIndexArray(Type type, int stride, void *data) = 0;
  virtual long EnableArray(ArrayType type, bool enabled) = 0;

  // vbo
  virtual long BindBuffer(VideoBuffer * buffer) = 0;
  virtual long BindIndexBuffer(VideoBuffer * buffer) = 0;
  virtual long UnbindBuffer() = 0;
  virtual long UnbindIndexBuffer() = 0;
  virtual long UpdateBuffer(VideoBuffer * buffer, void *data, uint size,
                            VideoBufferStoreMethod method) = 0;
  virtual long UpdateIndexBuffer(VideoBuffer * buffer, void *data, uint size,
                                 VideoBufferStoreMethod method) = 0;
  virtual void *MapBuffer(VideoBuffer * buffer, AccessMode access) = 0;
  virtual long UnmapBuffer(VideoBuffer * buffer) = 0;
  virtual void *MapIndexBuffer(VideoBuffer * buffer, AccessMode access) = 0;
  virtual long UnmapIndexBuffer(VideoBuffer * buffer) = 0;

  //////////////////////////////////////////////////////////////////////////shader
  // vertex shader
  virtual Shader *CreateVertexShader(const char *name) = 0;

  // pixel shader
  virtual Shader *CreatePixelShader(const char *name) = 0;

  virtual Shader *GetShader(const char *name) = 0;
  virtual long DestroyShader(const char *name) = 0;

  // program
  virtual Program *CreateProgram(const char *name) = 0;
  virtual Program *GetProgram(const char *name) = 0;
  virtual long DestroyProgram(const char *name) = 0;

  virtual long LoadShaderSource(Shader * shader, char *source) = 0;
  virtual long CompileShader(Shader * shader) = 0;
  virtual long IsShaderCompiled(Shader * shader) = 0;
  virtual char *GetShaderLog(Shader * shader) = 0;

  virtual long BindProgram(Program * program) = 0;
  virtual long UnbindProgram() = 0;
  virtual long SetProgramVertexShader(Program * program, Shader * shader) = 0;
  virtual long SetProgramPixelShader(Program * program, Shader * shader) = 0;
  virtual long LinkProgram(Program * program) = 0;
  virtual long IsProgramLinked(Program * program) = 0;
  virtual char *GetProgramLinkLog(Program * program) = 0;

  virtual long SetProgramVector(Program * program, string & param,
                                const Vector4 &value) = 0;
  virtual long SetProgramVector(Program * program, string & param,
                                const Vector3 &value) = 0;
  virtual long SetProgramVector(Program * program, string & param,
                                const Vector2 &value) = 0;
  virtual long SetProgramFloat(Program * program, string & param,
                               float value) = 0;
  virtual long SetProgramInt(Program * program, string & param, int value) = 0;
  virtual long GetProgramFloat(Program * program, string & param,
                               float *value) = 0;

  virtual long SetProgramTexture(Program * program, string & param,
                                 int texture) = 0;

  //////////////////////////////////////////////////////////////////////////texture
  // texture
  virtual Texture *CreateTexture(const char *name) = 0;
  virtual Texture *GetTexture(const char *name) = 0;
  virtual long DestroyTexture(const char *name) = 0;

  virtual long GenerateMipmap(Texture * texture) = 0;
  virtual long BindTexture(Texture * texture) = 0;
  virtual long BuildTexture(Texture * texture) = 0;
  virtual long BindRectTexture(Texture * texture) = 0;
  virtual long UnbindTexture() = 0;
  virtual long UnbindRectTexture() = 0;

  //////////////////////////////////////////////////////////////////////////
  // frame buffer
  virtual FrameBuffer *CreateFrameBuffer() = 0;
  virtual long DestroyFrameBuffer(FrameBuffer * frameBuffer) = 0;
  virtual long BindFrameBuffer(FrameBuffer * frameBuffer) = 0;
  virtual long UnbindFrameBuffer() = 0;

  virtual RenderBuffer *CreateRenderBuffer(TextureFormat format, uint width,
                                           uint height) = 0;
  virtual long DestroyRenderBuffer(RenderBuffer * renderBuffer) = 0;
  virtual long AttachRenderBuffer(FrameBuffer * frameBuffer,
                                  RenderBuffer * renderBuffer,
                                  RenderBufferSlot slot) = 0;
  virtual long AttachTexture(FrameBuffer * frameBuffer, Texture * texture2D,
                             RenderBufferSlot slot) = 0;
  virtual FrameBufferStatus CheckFrameBufferStatus() = 0;

  //////////////////////////////////////////////////////////////////////////font
  // font
  virtual long CreateFont(const char *chType, int nHeight, int nWidth,
                          int nWeight, bool bItalic, bool bUnderline,
                          bool bStrike, ulong unSize, uint &unID) = 0;

 public:
  // fast draw
  virtual long DrawCube3D(Vector3 vCenter, float fWidth, Color smtClr) = 0;

 protected:
  HINSTANCE dll_;
  RenderBase3DApi m_rBaseApi;
  MatrixMode m_matrixMode;
  Viewport3D m_viewPort;
  string m_strLogName;

 protected:
  ShaderManager m_shaderMgr;
  ProgramManager m_progamMgr;
  TextureManager m_textureMgr;

 protected:
  bool m_bBlending;
};

typedef class 3DRenderDevice * LP3DRENDERDEVICE;

extern "C" {
_declspec(dllexport) HRESULT
    Create3DRenderDevice(HINSTANCE hDLL, 3DRenderDevice * &pInterface);
typedef HRESULT (*_Create3DRenderDevice)(HINSTANCE hDLL,
                                         3DRenderDevice * &pInterface);

_declspec(dllexport) HRESULT
    Release3DRenderDevice(3DRenderDevice * &pInterface);
typedef HRESULT (*_Release3DRenderDevice)(3DRenderDevice * &pInterface);
}
}  // namespace _3Drd

#endif  //_RD3D_3DRENDERDEVICE_H
