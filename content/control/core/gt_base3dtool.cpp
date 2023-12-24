#include "gt_base3dtool.h"

namespace _GroupTool {
Base3DTool::Base3DTool() : m_p3DRenderDevice(NULL), m_pScene(NULL) { ; }

Base3DTool::~Base3DTool() {
  m_p3DRenderDevice = NULL;
  m_pScene = NULL;
}

int Base3DTool::Init(LP3DRENDERDEVICE p3DRenderDevice, Scene* pScene, HWND hWnd,
                     pfnToolCallBack pfnCallBack, void* to_follow) {
  if (ERR_NONE != IATool::Init(hWnd, pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  if (NULL == p3DRenderDevice || NULL == pScene) {
    return ERR_FAILURE;
  }

  m_p3DRenderDevice = p3DRenderDevice;
  m_pScene = pScene;

  return ERR_NONE;
}
}  // namespace _GroupTool