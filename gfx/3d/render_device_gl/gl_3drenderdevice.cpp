#include "gl_3drenderdevice.h"

#include "gl_fbofuncimp.h"
#include "gl_mipmapfuncimp.h"
#include "gl_multitexturefuncimp.h"
#include "gl_shaderfuncimp.h"
#include "gl_text.h"
#include "gl_vbofuncimp.h"
#include "gl_vsyncfuncimp.h"
#include "rd3d_programmanager.h"
#include "rd3d_shadermanager.h"
#include "rd3d_texturemanager.h"
#include "smt_logmanager.h"


using namespace core;

namespace _3Drd {
GLRenderDevice::GLRenderDevice()
    : m_pDeviceCaps(NULL),
      m_pStateManager(NULL),
      m_pFuncShaders(NULL),
      m_pFuncMultTex(NULL),
      m_pFuncVSync(NULL),
      m_pFuncMipmap(NULL) {
  m_rBaseApi = RA_OPENGL;
  dll_ = NULL;
  m_strLogName = "";
  m_pStateManager = new GLGPUStateManager();
}

GLRenderDevice::GLRenderDevice(HINSTANCE hDLL)
    : m_pDeviceCaps(NULL),
      m_pStateManager(NULL),
      m_pFuncShaders(NULL),
      m_pFuncMultTex(NULL),
      m_pFuncVSync(NULL),
      m_pFuncMipmap(NULL) {
  m_rBaseApi = RA_OPENGL;
  dll_ = hDLL;
  m_strLogName = "";
  m_pStateManager = new GLGPUStateManager();
}

GLRenderDevice::~GLRenderDevice() { Release(); }

long GLRenderDevice::Init(HWND hWnd, const char *logname) {
  assert(::IsWindow(hWnd));
  m_hWnd = hWnd;

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->CreateLog(logname);

  if (NULL == pLog) return FALSE;

  pLog->LogMessage("Init OpenGL 3DRenderDevice ok!");

  m_strLogName = logname;

  //////////////////////////////////////////////////////////////////////////
  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 16;
  pfd.iLayerType = PFD_MAIN_PLANE;

  HDC hDC = ::GetDC(m_hWnd);

  // Choose pixel format
  int nPixelFormat = ChoosePixelFormat(hDC, &pfd);
  if (nPixelFormat == 0) {
    return ERR_FAILURE;
  } else {
    // Set pixel format
    BOOL bResult = SetPixelFormat(hDC, nPixelFormat, &pfd);
    if (!bResult) {
      return ERR_FAILURE;
    } else {
      // Create a rendering context.
      m_hRC = wglCreateContext(hDC);
      if (!m_hRC) {
        return ERR_FAILURE;
      } else {
        // Set it as the current context
        if (!wglMakeCurrent(hDC, m_hRC)) {
          return ERR_FAILURE;
        }
      }
    }
  }

  ::ReleaseDC(m_hWnd, hDC);

  //////////////////////////////////////////////////////////////////////////
  if (ERR_NONE != SetDeviceCaps()) return ERR_FAILURE;

  //////////////////////////////////////////////////////////////////////////
  // Set a default viewport
  glEnable(GL_SCISSOR_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_ALPHA);

  glEnable(GL_LINE_SMOOTH);
  glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);

  RECT wndbfRect;
  Viewport3D viewport;
  GetClientRect(m_hWnd, &wndbfRect);

  viewport.ulHeight = wndbfRect.bottom - wndbfRect.top;
  viewport.ulWidth = wndbfRect.right - wndbfRect.left;
  viewport.ulX = 0;
  viewport.ulY = 0;
  viewport.fZNear = 0;
  viewport.fZFar = 1;
  viewport.fFovy = 45.f;
  SetViewport(viewport);

  SetClearColor(Color(1, 0, 0));
  SetDepthClearValue(1.0f);
  SetStencilClearValue(0);

  return ERR_NONE;
}

long GLRenderDevice::Destroy() {
  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetDefaultLog();

  //ÊÍ·Åopengl×ÊÔ´
  //////////////////////////////////////////////////////////////////////////
  vector<string> vStrResNames;
  m_textureMgr.GetAllTextureName(vStrResNames);
  for (int i = 0; i < vStrResNames.size(); i++) {
    Texture *pTexture = m_textureMgr.GetTexture(vStrResNames[i].c_str());
    if (pTexture) {
      GLhandleARB handle = pTexture->GetHandle();
      if (handle != 0) glDeleteTextures(1, &handle);
    }
  }
  m_textureMgr.DestroyAllTexture();

  //////////////////////////////////////////////////////////////////////////
  m_shaderMgr.GetAllShaderName(vStrResNames);
  for (int i = 0; i < vStrResNames.size(); i++) {
    Shader *pShader = m_shaderMgr.GetShader(vStrResNames[i].c_str());
    if (pShader) {
      GLhandleARB handle = pShader->GetHandle();
      if (handle != 0) m_pFuncShaders->glDeleteObject(handle);
    }
  }
  m_shaderMgr.DestroyAllShader();

  //////////////////////////////////////////////////////////////////////////
  m_progamMgr.GetAllProgramName(vStrResNames);
  for (int i = 0; i < vStrResNames.size(); i++) {
    Program *pProgram = m_progamMgr.GetProgram(vStrResNames[i].c_str());
    if (pProgram) {
      GLhandleARB handle = pProgram->GetHandle();
      if (handle != 0) m_pFuncShaders->glDeleteObject(handle);
    }
  }
  m_progamMgr.DestroyAllProgram();

  //////////////////////////////////////////////////////////////////////////

  if (::wglGetCurrentContext()) ::wglMakeCurrent(NULL, NULL);

  if (m_hRC) {
    ::wglDeleteContext(m_hRC);
    m_hRC = NULL;
  }

  //
  SAFE_DELETE(m_pDeviceCaps);

  //
  SAFE_DELETE(m_pFuncShaders);
  SAFE_DELETE(m_pFuncMultTex);
  SAFE_DELETE(m_pFuncVSync);
  SAFE_DELETE(m_pFuncMipmap);
  SAFE_DELETE(m_pFuncVBO);
  SAFE_DELETE(m_pFuncFBO);

  return ERR_NONE;
}

long GLRenderDevice::Release() {
  //
  SAFE_DELETE(m_pStateManager);
  //

  int nFont = m_vTextPtrs.size();
  for (int i = 0; i < nFont; i++) {
    GLText *pFont = m_vTextPtrs.at(i);
    SAFE_DELETE(pFont);
  }
  m_vTextPtrs.clear();

  return ERR_NONE;
}

long GLRenderDevice::SetDeviceCaps(void) {
  m_pDeviceCaps = new GLDeviceCaps(this);

  /* Init ARB_multitexture */
  if (m_pDeviceCaps->IsMultiTextureSupported())
    m_pFuncMultTex = new MultitextureFuncImpl();
  else
    m_pFuncMultTex = new MultitextureFunc();

  if (NULL != m_pFuncMultTex &&
      ERR_NONE == m_pFuncMultTex->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  /* Init GL_ARB_vertex_buffer_object */
  if (m_pDeviceCaps->IsVBOSupported())
    m_pFuncVBO = new VBOFuncImpl();
  else
    m_pFuncVBO = new VBOFunc();

  if (NULL != m_pFuncVBO && ERR_NONE == m_pFuncVBO->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  /* Init shaders */
  if (m_pDeviceCaps->IsGLSLSupported())
    m_pFuncShaders = new ShadersFuncImpl();
  else
    m_pFuncShaders = new ShadersFunc();

  if (NULL != m_pFuncShaders &&
      ERR_NONE == m_pFuncShaders->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  /* Init frame buffer objects */
  if (m_pDeviceCaps->IsFBOSupported())
    m_pFuncFBO = new FBOFuncImpl();
  else
    m_pFuncFBO = new FBOFunc();

  if (NULL != m_pFuncFBO && ERR_NONE == m_pFuncFBO->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  /* Init mimmap generation */
  if (m_pDeviceCaps->IsMipMapsSupported())
    m_pFuncMipmap = new MipmapFuncImpl();
  else
    m_pFuncMipmap = new MipmapFunc();

  if (NULL != m_pFuncMipmap && ERR_NONE == m_pFuncMipmap->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  /* Init VSync extension */
  if (m_pDeviceCaps->IsVSyncSupported())
    m_pFuncVSync = new VSyncFuncImpl();
  else
    m_pFuncVSync = new VSyncFunc();

  if (NULL != m_pFuncVSync && ERR_NONE == m_pFuncVSync->Initialize(this))
    ;
  else
    return ERR_FAILURE;

  return ERR_NONE;
}

GLenum GLRenderDevice::ConvertType(Type type) {
  switch (type) {
    case SHORT:
      return GL_SHORT;
    case INT:
      return GL_INT;
    case FLOAT:
      return GL_FLOAT;
    case DOUBLE:
      return GL_DOUBLE;
    case UNSIGNED_INT:
      return GL_UNSIGNED_INT;
    case UNSIGNED_BYTE:
      return GL_UNSIGNED_BYTE;
    case UNSIGNED_SHORT:
      return GL_UNSIGNED_SHORT;
  }
  return -1;
}

GLenum GLRenderDevice::ConvertVideoBufferStoreMethod(
    VideoBufferStoreMethod method) {
  switch (method) {
    case STATIC_DRAW:
      return GL_STATIC_DRAW_ARB;
    case STATIC_READ:
      return GL_STATIC_READ_ARB;
    case STATIC_COPY:
      return GL_STATIC_COPY_ARB;
    case DYNAMIC_DRAW:
      return GL_DYNAMIC_DRAW_ARB;
    case DYNAMIC_READ:
      return GL_DYNAMIC_READ_ARB;
    case DYNAMIC_COPY:
      return GL_DYNAMIC_COPY_ARB;
    case STREAM_DRAW:
      return GL_STREAM_DRAW_ARB;
    case STREAM_READ:
      return GL_STREAM_READ_ARB;
    case STREAM_COPY:
      return GL_STREAM_COPY_ARB;
  }

  return -1;
}

GLenum GLRenderDevice::ConvertAccess(AccessMode access) {
  switch (access) {
    case READ_ONLY:
      return GL_READ_ONLY_ARB;
    case WRITE_ONLY:
      return GL_WRITE_ONLY_ARB;
    case READ_WRITE:
      return GL_READ_WRITE_ARB;
  }
  return -1;
}

GLenum GLRenderDevice::ConvertArrayType(ArrayType type) {
  switch (type) {
    case TEXTURE_COORD_ARRAY:
      return GL_TEXTURE_COORD_ARRAY;
    case COLOR_ARRAY:
      return GL_COLOR_ARRAY;
    case INDEX_ARRAY:
      return GL_INDEX_ARRAY;
    case NORMAL_ARRAY:
      return GL_NORMAL_ARRAY;
    case VERTEX_ARRAY:
      return GL_VERTEX_ARRAY;
  }
  return -1;
}

GLenum GLRenderDevice::ConvertTexFormat(TextureFormat format) {
  switch (format) {
    case RGB8:
      return GL_RGB;
    case RGBA8:
      return GL_RGBA;
    case RGB_DXT1:
      return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case RGBA_DXT1:
      return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case RGBA_DXT3:
      return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case RGBA_DXT5:
      return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case LUMINANCE8:
      return GL_LUMINANCE;
    case INTENSITY8:
      return GL_INTENSITY;
    case RGB16F:
      return GL_RGB16F_ARB;
    case RGBA16F:
      return GL_RGBA16F_ARB;
    case ALPHA16F:
      return GL_ALPHA16F_ARB;
    case INTENSITY16F:
      return GL_INTENSITY16F_ARB;
    case LUMINANCE16F:
      return GL_LUMINANCE16F_ARB;
    case LUMINANCE_ALPHA16F:
      return GL_LUMINANCE_ALPHA16F_ARB;
    case RGB32F:
      return GL_RGB32F_ARB;
    case RGBA32F:
      return GL_RGBA32F_ARB;
    case ALPHA32F:
      return GL_ALPHA32F_ARB;
    case INTENSITY32F:
      return GL_INTENSITY32F_ARB;
    case LUMINANCE32F:
      return GL_LUMINANCE32F_ARB;
    case LUMINANCE_ALPHA32F:
      return GL_LUMINANCE_ALPHA32F_ARB;
  }

  return (GLenum)ERROR;
}

}  // namespace _3Drd