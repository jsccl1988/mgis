#include "gt_wsviewctrltool.h"

#include <assert.h>
#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "msvr_mapclient.h"
#include "msvr_mapservice.h"
#include "resource.h "
#include "smt_api.h"
#include "sys_sysmanager.h"
#include "t_iatoolmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;
using namespace _MapService;
using namespace _MapClient;

const string CST_STR_MAPWSVIEWCTRL_TOOL_NAME = "复合地图浏览";

namespace _GroupTool {
WSViewCtrlTool::WSViewCtrlTool()
    : m_bCaptured(FALSE),
      m_usFlashed(0),
      m_lZoomLevel(0),
      m_viewMode(VM_ZoomOff) {
  SetName(CST_STR_MAPWSVIEWCTRL_TOOL_NAME.c_str());
}

WSViewCtrlTool::~WSViewCtrlTool() { UnRegisterMessage(); }

int WSViewCtrlTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
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

  AppendFuncItems("放大", GT_MSG_WSVIEW_ZOOMIN, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("缩小", GT_MSG_WSVIEW_ZOOMOUT, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("移动", GT_MSG_WSVIEW_ZOOMMOVE, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("复位", GT_MSG_WSVIEW_ZOOMRESTORE, FIM_2DVIEW | FIM_2DMFMENU);
  AppendFuncItems("刷新", GT_MSG_WSVIEW_ZOOMREFRESH, FIM_2DVIEW | FIM_2DMFMENU);

  IATOOL_APPEND_MSG(GT_MSG_WSVIEW_ZOOMIN);
  IATOOL_APPEND_MSG(GT_MSG_WSVIEW_ZOOMOUT);
  IATOOL_APPEND_MSG(GT_MSG_WSVIEW_ZOOMMOVE);
  IATOOL_APPEND_MSG(GT_MSG_WSVIEW_ZOOMRESTORE);
  IATOOL_APPEND_MSG(GT_MSG_WSVIEW_ZOOMREFRESH);

  IATOOL_APPEND_MSG(GT_MSG_SET_WSVIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_GET_WSVIEW_MODE);
  IATOOL_APPEND_MSG(GT_MSG_SET_ZOOMLEVEL);
  IATOOL_APPEND_MSG(GT_MSG_GET_ZOOMLEVEL);

  RegisterMessage();

  return ERR_NONE;
}

int WSViewCtrlTool::AuxDraw() { return ERR_NONE; }

int WSViewCtrlTool::Timer() { return ERR_NONE; }

int WSViewCtrlTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.source_window != m_hWnd) {
    switch (nMessage) {
      case GT_MSG_WSVIEW_ZOOMRESTORE: {
        m_viewMode = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case GT_MSG_WSVIEW_ZOOMREFRESH: {
        m_viewMode = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
    }
  } else {
    switch (nMessage) {
      case GT_MSG_WSVIEW_ZOOMIN: {
        m_viewMode = VM_ZoomIn;
      } break;
      case GT_MSG_WSVIEW_ZOOMOUT: {
        m_viewMode = VM_ZoomOut;
      } break;
      case GT_MSG_WSVIEW_ZOOMMOVE: {
        m_viewMode = VM_ZoomMove;
      } break;
      case GT_MSG_WSVIEW_ZOOMRESTORE: {
        m_viewMode = VM_ZoomRestore;
        ZoomRestore();
      } break;
      case GT_MSG_WSVIEW_ZOOMREFRESH: {
        m_viewMode = VM_ZoomRefresh;
        ZoomRefresh();
      } break;
      case GT_MSG_SET_WSVIEW_MODE: {
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
      case GT_MSG_GET_WSVIEW_MODE: {
        *(ushort *)param.wparam = m_viewMode;
      } break;
      case GT_MSG_SET_ZOOMLEVEL: {
        m_lZoomLevel = *(long *)param.wparam;
      } break;
      case GT_MSG_GET_ZOOMLEVEL: {
        *(long *)param.wparam = m_lZoomLevel;
      } break;
    }

    SetActive();
  }

  return ERR_NONE;
}

int WSViewCtrlTool::SetCursor(void) {
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

int WSViewCtrlTool::LButtonDown(uint nFlags, lPoint point) {
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

int WSViewCtrlTool::MouseMove(uint nFlags, lPoint point) {
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

int WSViewCtrlTool::LButtonUp(uint nFlags, lPoint point) {
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

int WSViewCtrlTool::RButtonDown(uint nFlags, lPoint point) {
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

int WSViewCtrlTool::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  POINT pnt;
  pnt.x = point.x;
  pnt.y = point.y;
  ScreenToClient(m_hWnd, &pnt);

  if (zDelta < 0) {
    WSMap *pWSMap = dynamic_cast<WSMap *>(m_pOperMap);

    if (NULL != pWSMap) {
      fRect rctVP;
      fPoint center;
      fPoint offset;
      float x1, y1, x2, y2;
      m_pRenderDevice->DPToLP(pnt.x, pnt.y, x1, y1);

      pWSMap->GetZoomCenter(center);

      pWSMap->SetZoom(pWSMap->GetZoom() - 1);

      pWSMap->GetClientLBRect(rctVP);
      m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);

      m_pRenderDevice->DPToLP(pnt.x, pnt.y, x2, y2);

      offset.x = x2 - x1;
      offset.y = y2 - y1;

      center.x -= offset.x;
      center.y -= offset.y;

      m_pRenderDevice->ZoomMove(m_pOperMap, offset);
      pWSMap->SetZoom(pWSMap->GetZoom(), center);
      pWSMap->Sync2TileLayers();
    }
  } else {
    WSMap *pWSMap = dynamic_cast<WSMap *>(m_pOperMap);

    if (NULL != pWSMap) {
      fRect rctVP;
      fPoint center;
      fPoint offset;
      float x1, y1, x2, y2;

      m_pRenderDevice->DPToLP(pnt.x, pnt.y, x1, y1);

      pWSMap->GetZoomCenter(center);

      pWSMap->SetZoom(pWSMap->GetZoom() + 1);

      pWSMap->GetClientLBRect(rctVP);
      m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);

      m_pRenderDevice->DPToLP(pnt.x, pnt.y, x2, y2);

      offset.x = x2 - x1;
      offset.y = y2 - y1;

      center.x -= offset.x;
      center.y -= offset.y;

      m_pRenderDevice->ZoomMove(m_pOperMap, offset);
      pWSMap->SetZoom(pWSMap->GetZoom(), center);
      pWSMap->Sync2TileLayers();
    }
  }

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void WSViewCtrlTool::ZoomMove(short mouse_status, core::lPoint point) {
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

        if (m_pOperMap != NULL) {
          fRect rctVP;
          fPoint center;

          WSMap *pWSMap = dynamic_cast<WSMap *>(m_pOperMap);

          if (NULL != pWSMap) {
            pWSMap->GetZoomCenter(center);
            center.x -= doffset.x;
            center.y -= doffset.y;

            pWSMap->SetZoom(pWSMap->GetZoom(), center);
            pWSMap->Sync2TileLayers();

            pWSMap->GetClientLBRect(rctVP);

            m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);
          }
        }

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

void WSViewCtrlTool::ZoomIn(short mouse_status, core::lPoint point) {
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

        if (m_pOperMap != NULL) {
          WSMap *pWSMap = dynamic_cast<WSMap *>(m_pOperMap);

          if (NULL != pWSMap) {
            fRect rctVP;
            fPoint center;
            fPoint offset;
            float x1, y1, x2, y2;

            m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);

            pWSMap->GetZoomCenter(center);
            pWSMap->SetZoom(pWSMap->GetZoom() + 1);

            pWSMap->GetClientLBRect(rctVP);
            m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);

            m_pRenderDevice->DPToLP(point.x, point.y, x2, y2);

            offset.x = x2 - x1;
            offset.y = y2 - y1;

            center.x -= offset.x;
            center.y -= offset.y;

            m_pRenderDevice->ZoomMove(m_pOperMap, offset);
            pWSMap->SetZoom(pWSMap->GetZoom(), center);
            pWSMap->Sync2TileLayers();
          }
        }
      }
    } break;
    case typeRButtonDown: {
      m_pntCur = point;
      if (m_bCaptured) {
        m_bCaptured = FALSE;
        ReleaseCapture();
      }

      m_viewMode = VM_ZoomOff;
    } break;
  }
}

void WSViewCtrlTool::ZoomOut(short mouse_status, core::lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown:
      break;
    case typeLButtonUp: {
      if (m_pOperMap != NULL) {
        fRect rctVP;
        WSMap *pWSMap = dynamic_cast<WSMap *>(m_pOperMap);

        if (NULL != pWSMap) {
          fRect rctVP;
          fPoint center;
          fPoint offset;
          float x1, y1, x2, y2;

          m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);

          pWSMap->GetZoomCenter(center);
          pWSMap->SetZoom(pWSMap->GetZoom() - 1);

          pWSMap->GetClientLBRect(rctVP);
          m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);

          m_pRenderDevice->DPToLP(point.x, point.y, x2, y2);

          offset.x = x2 - x1;
          offset.y = y2 - y1;

          center.x -= offset.x;
          center.y -= offset.y;

          m_pRenderDevice->ZoomMove(m_pOperMap, offset);
          pWSMap->SetZoom(pWSMap->GetZoom(), center);
          pWSMap->Sync2TileLayers();
        }
      }
    } break;
    case typeMouseMove:
      break;
    case typeRButtonDown: {
      m_viewMode = VM_ZoomOff;
    } break;
  }
}

void WSViewCtrlTool::ZoomRestore() {
  if (m_pOperMap != NULL) {
    WSMap *pWSMap = (WSMap *)m_pOperMap;

    fRect rctVP;
    fPoint center;
    Envelope envMap;

    pWSMap->GetEnvelope(envMap);
    center.x = (envMap.MinX + envMap.MaxX) / 2.;
    center.y = (envMap.MinY + envMap.MaxY) / 2.;

    pWSMap->SetZoom(pWSMap->GetZoom(), center);
    pWSMap->Sync2TileLayers();

    pWSMap->GetClientLBRect(rctVP);

    m_pRenderDevice->ZoomToRect(m_pOperMap, rctVP);
  }
}

void WSViewCtrlTool::ZoomRefresh() {
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
