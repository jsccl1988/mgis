#include "gt_flashtool.h"

#include <assert.h>
#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "gis_api.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "resource.h "
#include "sde_datasourcemgr.h"
#include "smt_api.h"
#include "sys_sysmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace _SDEDevMgr;
using namespace _Sys;

const string CST_STR_FLASH_TOOL_NAME = "ÉÁË¸";

namespace _GroupTool {
FlashTool::FlashTool()
    : m_bStyle1(true), m_pResultLayer(NULL), m_fScaleDelt(0.15) {
  SetName(CST_STR_FLASH_TOOL_NAME.c_str());
}

FlashTool::~FlashTool() {
  DataSourceMgr::DestoryMemVecLayer(m_pResultLayer);

  UnRegisterMessage();
}

int FlashTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
                    pfnToolCallBack pfnCallBack, void *to_follow) {
  if (ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  SysManager *pSysMgr = SysManager::GetSingletonPtr();

  SystemOptions sysPra = pSysMgr->GetSysPra();

  m_fScaleDelt = sysPra.zoom_scale_delt;

  m_pResultLayer = DataSourceMgr::CreateMemVecLayer();
  m_pResultLayer->Open("");

  AppendFuncItems("¿ªÊ¼ÉÁË¸", GT_MSG_START_FLASH, FIM_2DVIEW);
  AppendFuncItems("Í£Ö¹ÉÁË¸", GT_MSG_STOP_FLASH, FIM_2DVIEW);

  IATOOL_APPEND_MSG(GT_MSG_START_FLASH);
  IATOOL_APPEND_MSG(GT_MSG_STOP_FLASH);

  IATOOL_APPEND_MSG(GT_MSG_SET_FLASH_MODE);
  IATOOL_APPEND_MSG(GT_MSG_GET_FLASH_MODE);
  IATOOL_APPEND_MSG(GT_MSG_SET_PRA);
  IATOOL_APPEND_MSG(GT_MSG_SET_FLASH_DATA);

  RegisterMessage();

  return ERR_NONE;
}

int FlashTool::AuxDraw() {
  if (m_bFlash) {
    StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();

    Style *pStyle = pStyleMgr->GetStyle(m_strFlashStyle.c_str());
    if (ERR_NONE == m_pRenderDevice->BeginRender(MRD_BL_DYNAMIC, true,
                                                     pStyle, R2_COPYPEN)) {
      m_pRenderDevice->RenderLayer(m_pResultLayer, R2_COPYPEN);
      m_pRenderDevice->EndRender(MRD_BL_DYNAMIC);
    }
  }

  return ERR_NONE;
}

int FlashTool::Timer() {
  m_bStyle1 = !m_bStyle1;
  if (m_bStyle1)
    m_strFlashStyle = m_strFlashStyle1;
  else
    m_strFlashStyle = m_strFlashStyle2;

  return ERR_NONE;
}

int FlashTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.source_window != m_hWnd) return ERR_NONE;

  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
    } break;
    case GT_MSG_STOP_FLASH: {
      m_bFlash = false;

      if (ERR_NONE == m_pRenderDevice->BeginRender(MRD_BL_DYNAMIC, true,
                                                       NULL, R2_COPYPEN)) {
        m_pRenderDevice->RenderLayer((Layer *)NULL, R2_COPYPEN);
        m_pRenderDevice->EndRender(MRD_BL_DYNAMIC);
      }

      m_pRenderDevice->Refresh();
    } break;
    case GT_MSG_START_FLASH: {
      m_bFlash = true;
    } break;
    case GT_MSG_SET_FLASH_MODE: {
      m_flsMode = eFlashMode(*(ushort *)param.wparam);
    } break;
    case GT_MSG_GET_FLASH_MODE: {
      *(ushort *)param.wparam = m_flsMode;
    } break;
    case GT_MSG_GET_STATUS: {
      *(ushort *)param.wparam = (m_bFlash) ? 1 : 0;
    } break;
    case GT_MSG_SET_PRA: {
      //
    } break;
    case GT_MSG_SET_FLASH_DATA: {
      //
      int nLayerFeaType = *(int *)param.lparam;
      VectorLayer *pSrcLayer = (VectorLayer *)param.wparam;
      m_pResultLayer->DeleteAll();
      CopyLayer(m_pResultLayer, pSrcLayer);

      SysManager *pSysMgr = SysManager::GetSingletonPtr();

      switch (nLayerFeaType) {
        case FeatureType::FtChildImage:
        case FeatureType::FtDot:
        case FeatureType::FtAnno: {
          m_strFlashStyle1 = pSysMgr->GetSysStyleConfig().dot_flash_style1_;
          m_strFlashStyle2 = pSysMgr->GetSysStyleConfig().dot_flash_style2_;
        } break;
        case FeatureType::FtCurve: {
          m_strFlashStyle1 = pSysMgr->GetSysStyleConfig().line_flash_style1_;
          m_strFlashStyle2 = pSysMgr->GetSysStyleConfig().line_flash_style2_;
        } break;
        case FeatureType::FtSurface: {
          m_strFlashStyle1 = pSysMgr->GetSysStyleConfig().region_flash_style1_;
          m_strFlashStyle2 = pSysMgr->GetSysStyleConfig().region_flash_style2_;
        } break;
      }
    } break;
    default:
      break;
  }
  return ERR_NONE;
}
}  // namespace _GroupTool
