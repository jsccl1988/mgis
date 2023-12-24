#include "gt_viewctrltool.h"

#include <assert.h>
#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "resource.h "
#include "smt_api.h"
#include "sys_sysmanager.h"
#include "t_iatoolmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;

const string CST_STR_MAPVIEWCTRL_TOOL_NAME = "地图浏览";

namespace _GroupTool {
ViewCtrlTool::ViewCtrlTool()
    : m_bCaptured(FALSE), m_usFlashed(0), m_viewMode(VM_ZoomOff) {
  SetName(CST_STR_MAPVIEWCTRL_TOOL_NAME.c_str());
}

ViewCtrlTool::~ViewCtrlTool() { UnRegisterMessage(); }

int ViewCtrlTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
                       HWND hWnd, pfnToolCallBack pfnCallBack,
                       void *to_follow) {
  if (ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  UINT idCursors[] = {IDC_CURSOR_ZOOMIN, IDC_CURSOR_ZOOMOUT,
                      IDC_CURSOR_ZOOMMOVE, IDC_CURSOR_IDENTIFY};

  int nCount = sizeof(idCursors) / sizeof(UINT);

  for (int i = 0; i < nCount; i++)
    m_hCursors[i] = ::LoadCursor(g_hInstance, MAKEINTRESOURCE(idCursors[i]));

  AppendFunctionItems("放大", GT_MSG_VIEW_ZOOMIN, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFunctionItems("缩小", GT_MSG_VIEW_ZOOMOUT, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFunctionItems("移动", GT_MSG_VIEW_ZOOMMOVE, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFunctionItems("复位", GT_MSG_VIEW_ZOOMRESTORE, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFunctionItems("刷新", GT_MSG_VIEW_ZOOMREFRESH, FIM_2DVIEW | FIM_2DMFMENU);
  // AppendFunctionItems("三维视图",GT_MSG_3DVIEW_ACTIVE,FIM_2DVIEW);

  IATOOL_APPEND_MSG(GT_MSG_VIEW_ZOOMIN);
  IATOOL_APPEND_MSG(GT_MSG_VIEW_ZOOMOUT);
  IATOOL_APPEND_MSG(GT_MSG_VIEW_ZOOMMOVE);
  IATOOL_APPEND_MSG(GT_MSG_VIEW_ZOOMRESTORE);
  IATOOL_APPEND_MSG(GT_MSG_VIEW_ZOOMREFRESH);
  // IATOOL_APPEND_MSG(GT_MSG_VIEW_ACTIVE);

  IATOOL_APPEND_MSG(GT_MSG_SET_VIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_GET_VIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_SET_SCALEDELT);
  IATOOL_APPEND_MSG(GT_MSG_GET_SCALEDELT);

  RegisterMessage();

  return ERR_NONE;
}

int ViewCtrlTool::AuxDraw() { return ERR_NONE; }

int ViewCtrlTool::Timer() { return ERR_NONE; }

int ViewCtrlTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.source_window != m_hWnd) {
    switch (nMessage) {
      case GT_MSG_VIEW_ZOOMRESTORE: {
        m_viewMode = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case GT_MSG_VIEW_ZOOMREFRESH: {
        m_viewMode = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
      case GT_MSG_VIEW_ACTIVE: {
        SetForegroundWindow(m_hWnd);
      } break;
    }
  } else {
    switch (nMessage) {
      case GT_MSG_VIEW_ZOOMIN: {
        m_viewMode = VM_ZoomIn;
      } break;
      case GT_MSG_VIEW_ZOOMOUT: {
        m_viewMode = VM_ZoomOut;
      } break;
      case GT_MSG_VIEW_ZOOMMOVE: {
        m_viewMode = VM_ZoomMove;
      } break;
      case GT_MSG_VIEW_ZOOMRESTORE: {
        m_viewMode = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case GT_MSG_VIEW_ZOOMREFRESH: {
        m_viewMode = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
      case GT_MSG_SET_VIEW_MODE: {
        m_viewMode = eViewMode(*(ushort *)param.wparam);

        switch (m_viewMode) {
          case VM_ZoomRestore:
            ZoomRestore();
            break;
          case VM_ZoomRefresh:
            ZoomRefresh();
            break;
          default:
            break;
        }
      } break;
      case GT_MSG_GET_VIEW_MODE: {
        *(ushort *)param.wparam = m_viewMode;
      } break;
      case GT_MSG_SET_SCALEDELT: {
        m_fScaleDelt = *(double *)param.wparam;
      } break;
      case GT_MSG_GET_SCALEDELT: {
        *(double *)param.wparam = m_fScaleDelt;
      } break;
    }

    SetActive();
  }

  return ERR_NONE;
}

int ViewCtrlTool::SetCursor(void) {
  switch (m_viewMode) {  // Zoom mode select
    case VM_ZoomOff:
      ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
      break;
    case VM_ZoomIn:
      ::SetCursor(m_hCursors[CursorLoupePlus]);
      break;
    case VM_ZoomOut:
      ::SetCursor(m_hCursors[CursorLoupeMinus]);
      break;
    case VM_ZoomMove:
      ::SetCursor(m_hCursors[CursorMove]);
      break;
    case VM_ZoomRestore:
      ::SetCursor(m_hCrossCursor);
      break;
    case VM_ZoomRefresh:
      ::SetCursor(m_hCrossCursor);
      break;
    default:
      // All other zoom modes
      ::SetCursor(m_hCrossCursor);
      break;
  }

  return ERR_NONE;
}

int ViewCtrlTool::LButtonDown(uint nFlags, lPoint point) {
  SetOperDone(false);

  switch (m_viewMode) {
    case VM_ZoomMove:
      ZoomMove(typeLButtonDown, point);
      break;
    case VM_ZoomIn:
      ZoomIn(typeLButtonDown, point);
      break;
    case VM_ZoomOut:
      ZoomOut(typeLButtonDown, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int ViewCtrlTool::MouseMove(uint nFlags, lPoint point) {
  switch (m_viewMode) {
    case VM_ZoomMove:
      ZoomMove(typeMouseMove, point);
      break;
    case VM_ZoomIn:
      ZoomIn(typeMouseMove, point);
      break;
    case VM_ZoomOut:
      ZoomOut(typeMouseMove, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int ViewCtrlTool::LButtonUp(uint nFlags, lPoint point) {
  switch (m_viewMode) {
    case VM_ZoomMove:
      ZoomMove(typeLButtonUp, point);
      break;
    case VM_ZoomIn:
      ZoomIn(typeLButtonUp, point);
      break;
    case VM_ZoomOut:
      ZoomOut(typeLButtonUp, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int ViewCtrlTool::RButtonDown(uint nFlags, lPoint point) {
  switch (m_viewMode) {
    case VM_ZoomMove:
      ZoomMove(typeRButtonDown, point);
      break;
    case VM_ZoomIn:
      ZoomIn(typeRButtonDown, point);
      break;
    case VM_ZoomOut:
      ZoomOut(typeRButtonDown, point);
      break;
    default:
      break;
  }

  SetOperDone(true);

  return ERR_NONE;
}

int ViewCtrlTool::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  float fScale;
  if (zDelta < 0) {
    fScale = 1 + m_fScaleDelt;
  } else
    fScale = 1 - m_fScaleDelt;

  POINT pnt;
  pnt.x = point.x;
  pnt.y = point.y;
  ScreenToClient(m_hWnd, &pnt);

  lPoint lpnt(pnt.x, pnt.y);
  m_pRenderDevice->ZoomScale(m_pOperMap, lpnt, fScale);
  m_pRenderDevice->Refresh();

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void ViewCtrlTool::ZoomMove(short mouse_status, core::lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown: {
      m_bCaptured = TRUE;
      SetCapture(m_hWnd);
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      ListenerMessage param;
      param.source_window = m_hWnd;
      param.wparam = (WPARAM)(&m_usFlashed);
      PostIAToolMessage(IATOOL_MSG_BROADCAST,
                    IATOOL_MSG_KEY(GT_MSG_GET_STATUS), param);

      if (*(ushort *)param.wparam) {
        PostIAToolMessage(IATOOL_MSG_BROADCAST,
                      IATOOL_MSG_KEY(GT_MSG_STOP_FLASH), param);
      }
    } break;
    case typeMouseMove: {
      m_pntPrev = m_pntCur;
      m_pntCur = point;

      if (m_bCaptured) {
        lPoint curOrgPos;
        curOrgPos.x = m_pntCur.x - m_pntOrigin.x;
        curOrgPos.y = m_pntCur.y - m_pntOrigin.y;

        m_pRenderDevice->SetCurDrawingOrg(curOrgPos);
        m_pRenderDevice->Refresh();
      }
    } break;
    case typeLButtonUp: {
      m_pntPrev = m_pntCur;
      m_pntCur = point;
      if (m_bCaptured) {
        m_bCaptured = FALSE;
        ReleaseCapture();

        lPoint curOrgPos;
        curOrgPos.x = 0;
        curOrgPos.y = 0;

        m_pRenderDevice->SetCurDrawingOrg(curOrgPos);

        float x1, y1, x2, y2;
        m_pRenderDevice->DPToLP(m_pntOrigin.x, m_pntOrigin.y, x1, y1);
        m_pRenderDevice->DPToLP(point.x, point.y, x2, y2);

        fPoint doffset(x2 - x1, y2 - y1);
        m_pRenderDevice->ZoomMove(m_pOperMap, doffset);
        m_pRenderDevice->Refresh();

        if (m_usFlashed) {
          ListenerMessage param;
          param.source_window = m_hWnd;

          PostIAToolMessage(IATOOL_MSG_BROADCAST,
                        IATOOL_MSG_KEY(GT_MSG_START_FLASH), param);
        }
      }
    } break;
    case typeRButtonDown: {
      if (m_bCaptured) {
        m_bCaptured = FALSE;
        ReleaseCapture();
      }

      m_viewMode = VM_ZoomOff;
    } break;
    default:
      break;
  }
}

void ViewCtrlTool::ZoomIn(short mouse_status, core::lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown: {
      m_bCaptured = TRUE;
      SetCapture(m_hWnd);
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;
    } break;
    case typeLButtonUp: {
      if (m_bCaptured) {
        m_pntPrev = m_pntCur;
        m_pntCur = point;
        m_bCaptured = FALSE;
        ReleaseCapture();
        if (m_pntOrigin != point) {
          fRect frt;
          lRect lrt;
          lrt.lb.x = min(m_pntOrigin.x, m_pntCur.x);
          lrt.lb.y = std::max(m_pntOrigin.y, m_pntCur.y);
          lrt.rt.x = std::max(m_pntOrigin.x, m_pntCur.x);
          lrt.rt.y = min(m_pntOrigin.y, m_pntCur.y);

          m_pRenderDevice->DRectToLRect(lrt, frt);
          m_pRenderDevice->ZoomToRect(m_pOperMap, frt);
        } else
          m_pRenderDevice->ZoomScale(m_pOperMap, point, 1 - m_fScaleDelt);

        if (ERR_NONE ==
            m_pRenderDevice->BeginRender(MRD_BL_QUICK, true, false))
          m_pRenderDevice->EndRender(MRD_BL_QUICK);

        m_pRenderDevice->Refresh();
      }
    } break;
    case typeMouseMove: {
      m_pntPrev = m_pntCur;
      m_pntCur = point;
      if (m_bCaptured) {
        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (ERR_NONE == m_pRenderDevice->BeginRender(
                                MRD_BL_QUICK, false, pStyle, R2_NOTXORPEN)) {
          fRect frt1, frt2;
          frt1.Merge(m_pntOrigin.x, m_pntOrigin.y);
          frt1.Merge(m_pntPrev.x, m_pntPrev.y);
          m_pRenderDevice->DrawRect(frt1, true);

          frt2.Merge(m_pntOrigin.x, m_pntOrigin.y);
          frt2.Merge(m_pntCur.x, m_pntCur.y);
          m_pRenderDevice->DrawRect(frt2, true);

          m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }
      }
    } break;
    case typeRButtonDown: {
      m_pntCur = point;
      if (m_bCaptured) {
        m_bCaptured = FALSE;
        ReleaseCapture();

        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (ERR_NONE == m_pRenderDevice->BeginRender(
                                MRD_BL_QUICK, false, pStyle, R2_NOTXORPEN)) {
          fRect frt;
          frt.Merge(m_pntOrigin.x, m_pntOrigin.y);
          frt.Merge(m_pntCur.x, m_pntCur.y);

          m_pRenderDevice->DrawRect(frt, true);

          m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }
      }

      m_viewMode = VM_ZoomOff;
    } break;
  }
}

void ViewCtrlTool::ZoomOut(short mouse_status, core::lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown:
      break;
    case typeLButtonUp: {
      m_pRenderDevice->ZoomScale(m_pOperMap, point, 1 + m_fScaleDelt);
      m_pRenderDevice->Refresh();
    } break;
    case typeMouseMove:
      break;
    case typeRButtonDown: {
      m_viewMode = VM_ZoomOff;
    } break;
  }
}

void ViewCtrlTool::ZoomRestore() {
  if (m_pOperMap != NULL) {
    Envelope envelope;
    fRect frt;

    Layer *pLayer = m_pOperMap->GetActiveLayer();
    if (pLayer) {
      pLayer->CalEnvelope();
      pLayer->GetEnvelope(envelope);
      EnvelopeToRect(frt, envelope);

      float fWidthDiv = frt.Width() / 40;
      float fHeightDiv = frt.Height() / 40;

      frt.rt.x += fWidthDiv;
      frt.rt.y += fHeightDiv;
      frt.lb.x -= fWidthDiv;
      frt.lb.y -= fHeightDiv;

      m_pRenderDevice->ZoomToRect(m_pOperMap, frt);
      m_pRenderDevice->Refresh();
    }
  }
}

void ViewCtrlTool::ZoomRefresh() {
  fRect frt;
  if (m_pOperMap != NULL) {
    Envelope envelope;
    fRect frt;

    Layer *pLayer = m_pOperMap->GetActiveLayer();
    if (pLayer) {
      pLayer->CalEnvelope();
      pLayer->GetEnvelope(envelope);
      EnvelopeToRect(frt, envelope);
      m_pRenderDevice->Refresh(m_pOperMap, frt);
    }
  }
}
}  // namespace _GroupTool
