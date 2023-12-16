﻿#include "gt_selecttool.h"

#include <assert.h>
#include <math.h>

#include "bl_stylemanager.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "gt_grouptoolfactory.h"
#include "resource.h "
#include "sde_datasourcemgr.h"
#include "smt_api.h"
#include "smt_gui_api.h"
#include "smt_logmanager.h"
#include "sys_sysmanager.h"
#include "t_iatoolmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;
using namespace _SDEDevMgr;

const string CST_STR_SELECT_TOOL_NAME = "选取";
const string C_STR_SELECT_TOO_LOG = "SelectTool";

namespace _GroupTool {
SelectTool::SelectTool() : m_bCaptured(false), m_pResultLayer(NULL) {
  SetName(CST_STR_SELECT_TOOL_NAME.c_str());
}

SelectTool::~SelectTool() {
  SMT_SAFE_DELETE(m_gQDes.pQueryGeom);

  this->EndDelegate();

  DataSourceMgr::DestoryMemVecLayer(m_pResultLayer);

  UnRegisterMessage();
}

int SelectTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
                     pfnToolCallBack pfnCallBack, void *pToFollow) {
  if (SMT_ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, pToFollow)) {
    return SMT_ERR_FAILURE;
  }

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetLog(C_STR_SELECT_TOO_LOG);
  if (NULL == pLog) {
    pLogMgr->CreateLog(C_STR_SELECT_TOO_LOG.c_str());
  }

  m_pResultLayer = DataSourceMgr::CreateMemVecLayer();
  m_pResultLayer->Open("");

  SysManager *pSysMgr = SysManager::GetSingletonPtr();

  SystemOptions sysPra = pSysMgr->GetSysPra();

  m_dpMargin = sysPra.select_margin;

  AppendFuncItems("点选", GT_MSG_SELECT_POINTSEL, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("框选", GT_MSG_SELECT_RECTSEL, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("多边形选取", GT_MSG_SELECT_POLYGONSEL,
                  FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("清除选择要素", GT_MSG_SELECT_CLEAR,
                  FIM_2DVIEW | FIM_2DMFMENU);

  SMT_IATOOL_APPEND_MSG(GT_MSG_SELECT_POINTSEL);
  SMT_IATOOL_APPEND_MSG(GT_MSG_SELECT_RECTSEL);
  SMT_IATOOL_APPEND_MSG(GT_MSG_SELECT_POLYGONSEL);
  SMT_IATOOL_APPEND_MSG(GT_MSG_SELECT_CLEAR);

  SMT_IATOOL_APPEND_MSG(GT_MSG_SET_SEL_MODE);
  SMT_IATOOL_APPEND_MSG(GT_MSG_GET_SEL_MODE);

  RegisterMessage();

  return SMT_ERR_NONE;
}

int SelectTool::AuxDraw() { return BaseTool::AuxDraw(); }

int SelectTool::Timer() { return BaseTool::AuxDraw(); }

int SelectTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.hSrcWnd != m_hWnd) return SMT_ERR_NONE;

  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
    } break;
    case GT_MSG_SELECT_POINTSEL: {
      m_selMode = ST_Point;

      OnSetSelMode();
      param.bModify = true;

      SetActive();
    } break;
    case GT_MSG_SELECT_RECTSEL: {
      m_selMode = ST_Rect;

      OnSetSelMode();
      param.bModify = true;

      SetActive();
    } break;
    case GT_MSG_SELECT_POLYGONSEL: {
      m_selMode = ST_Polygon;

      OnSetSelMode();
      param.bModify = true;

      SetActive();
    } break;
    case GT_MSG_SELECT_CLEAR: {
      m_pResultLayer->DeleteAll();

      ListenerMessage param;
      param.hSrcWnd = m_hWnd;
      param.wParam = WPARAM(m_pResultLayer);
      param.lParam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                    SMT_IATOOL_MSG_KEY(GT_MSG_SET_FLASH_DATA), param);

      SetActive();
    } break;

    case GT_MSG_SET_SEL_MODE: {
      m_selMode = eSelectMode(*(ushort *)param.wParam);

      OnSetSelMode();

      param.bModify = true;
    } break;
    case GT_MSG_GET_SEL_MODE: {
      *(ushort *)param.wParam = m_selMode;
    } break;
    case GT_MSG_RET_DELEGATE: {
      ushort uRetType = *(ushort *)param.lParam;
      m_gQDes.pQueryGeom = ((Geometry *)param.wParam)->Clone();

      OnRetDelegate(uRetType);
    } break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

void SelectTool::OnRetDelegate(int nRetType) {
  switch (nRetType) {
    case GT_MSG_RET_INPUT_POINT: {
      if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000))
        m_pResultLayer->DeleteAll();

      m_gQDes.select_margin = m_dpMargin / m_pRenderDevice->GetBlc();
      m_pOperMap->QueryFeature(&m_gQDes, &m_pQDes, m_pResultLayer,
                               m_nLayerFeaType);

      SMT_SAFE_DELETE(m_gQDes.pQueryGeom);

      ListenerMessage param;
      param.hSrcWnd = m_hWnd;
      param.wParam = WPARAM(m_pResultLayer);
      param.lParam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                    SMT_IATOOL_MSG_KEY(GT_MSG_SET_FLASH_DATA), param);

      param.wParam = param.lParam = NULL;
      PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                    SMT_IATOOL_MSG_KEY(GT_MSG_START_FLASH), param);

      if (m_pResultLayer->GetFeatureCount() == 1) {
        Show2DFeatureInfoDlg(m_pResultLayer->GetFeature(0));
      }
    } break;
    case GT_MSG_RET_INPUT_LINE: {
      if (!(GetAsyncKeyState(VK_LCONTROL) & 0x8000))
        m_pResultLayer->DeleteAll();

      m_gQDes.select_margin = m_dpMargin / m_pRenderDevice->GetBlc();
      m_pOperMap->QueryFeature(&m_gQDes, &m_pQDes, m_pResultLayer,
                               m_nLayerFeaType);

      SMT_SAFE_DELETE(m_gQDes.pQueryGeom);

      ListenerMessage param;
      param.hSrcWnd = m_hWnd;
      param.wParam = WPARAM(m_pResultLayer);
      param.lParam = LPARAM(&m_nLayerFeaType);
      PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                    SMT_IATOOL_MSG_KEY(GT_MSG_SET_FLASH_DATA), param);

      param.wParam = param.lParam = NULL;
      PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                    SMT_IATOOL_MSG_KEY(GT_MSG_START_FLASH), param);

      uint unID = SMT_C_INVALID_UINT_VALUE;

      if (m_pResultLayer->GetFeatureCount() > 1) {
        vector<uint> vIDs;

        m_pResultLayer->MoveFirst();
        while (!m_pResultLayer->IsEnd()) {
          Feature *pFea = m_pResultLayer->GetFeature();
          vIDs.push_back((uint)pFea->GetID());

          m_pResultLayer->MoveNext();
        }

        SelectOneDlg(unID, vIDs);
        Show2DFeatureInfoDlg(m_pResultLayer->GetFeatureByID(unID));
      } else if (m_pResultLayer->GetFeatureCount() == 1) {
        m_pResultLayer->MoveFirst();

        Show2DFeatureInfoDlg(m_pResultLayer->GetFeature());
      }
    } break;
    case GT_MSG_RET_INPUT_REGION: {
    } break;
  }
}

void SelectTool::OnSetSelMode(void) {
  SysManager *pSysMgr = SysManager::GetSingletonPtr();
  StyleConfig &styleSonfig = pSysMgr->GetSysStyleConfig();

  this->EndDelegate();

  BaseTool *pInputTool = NULL;

  switch (m_selMode) {
    case ST_Point: {
      GroupToolFactory::CreateGroupTool(pInputTool,
                                        GroupToolType::GTT_InputPoint);

      if (NULL != pInputTool) {
        pInputTool->SetToolStyleName(styleSonfig.curve_style_);

        if (SMT_ERR_NONE ==
            pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
          ushort unPointType = PT_DOT;

          ListenerMessage param;
          param.hSrcWnd = m_hWnd;
          param.wParam = WPARAM(&unPointType);
          param.lParam = NULL;
          pInputTool->Notify(GT_MSG_SET_INPUT_POINT_TYPE, param);

          this->BeginDelegate(pInputTool);
        }
      }
    } break;
    case ST_Polygon: {
      GroupToolFactory::CreateGroupTool(pInputTool,
                                        GroupToolType::GTT_InputLine);

      if (NULL != pInputTool) {
        pInputTool->SetToolStyleName(styleSonfig.curve_style_);

        if (SMT_ERR_NONE ==
            pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
          ushort unLineType = LT_LinearRing;

          ListenerMessage param;
          param.hSrcWnd = m_hWnd;
          param.wParam = WPARAM(&unLineType);
          param.lParam = NULL;

          pInputTool->Notify(GT_MSG_SET_INPUT_LINE_TYPE, param);

          this->BeginDelegate(pInputTool);
        }
      }
    } break;
    case ST_Rect: {
      GroupToolFactory::CreateGroupTool(pInputTool,
                                        GroupToolType::GTT_InputLine);

      if (NULL != pInputTool) {
        pInputTool->SetToolStyleName(styleSonfig.curve_style_);

        if (SMT_ERR_NONE ==
            pInputTool->Init(m_pRenderDevice, m_pOperMap, m_hWnd)) {
          ushort unLineType = LT_Rect;

          ListenerMessage param;
          param.hSrcWnd = m_hWnd;
          param.wParam = WPARAM(&unLineType);
          param.lParam = NULL;

          pInputTool->Notify(GT_MSG_SET_INPUT_LINE_TYPE, param);

          this->BeginDelegate(pInputTool);
        }
      }
    } break;
  }
}

int SelectTool::KeyDown(uint nChar, uint nRepCnt, uint nFlags) {
  if (m_pDelegateTag != NULL && !m_pDelegateTag->IsOperDone())
    return m_pDelegateTag->KeyDown(nChar, nRepCnt, nFlags);
  else {
    if (!(GetKeyState(VK_CONTROL) & 0x8000)) {
      switch (nChar) {
        case 'c':
        case 'C': {
          m_pResultLayer->DeleteAll();

          ListenerMessage param;
          param.hSrcWnd = m_hWnd;
          param.wParam = WPARAM(m_pResultLayer);
          param.lParam = LPARAM(&m_nLayerFeaType);
          PostIAToolMessage(SMT_IATOOL_MSG_BROADCAST,
                        SMT_IATOOL_MSG_KEY(GT_MSG_SET_FLASH_DATA), param);

          SetActive();
        } break;
      }
    }
  }

  return BaseTool::KeyDown(nChar, nRepCnt, nFlags);
}
}  // namespace _GroupTool
