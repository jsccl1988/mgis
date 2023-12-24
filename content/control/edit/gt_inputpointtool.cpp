#include "gt_inputpointtool.h"

#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "geo_geometry.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "gt_defs.h"
#include "resource.h "
#include "smt_api.h"
#include "sys_sysmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;

const string CST_STR_INPUTPOINT_TOOL_NAME = "ÊäÈëµã";

namespace _GroupTool {
InputPointTool::InputPointTool()
    : m_pGeom(NULL),
      m_bIsDrag(false),
      m_bDelay(false),
      m_appendType(-1),
      m_fAngle(0.) {
  SetName(CST_STR_INPUTPOINT_TOOL_NAME.c_str());
}

InputPointTool::~InputPointTool() {
  SAFE_DELETE(m_pGeom);

  UnRegisterMessage();
}

int InputPointTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
                         HWND hWnd, pfnToolCallBack pfnCallBack,
                         void *to_follow) {
  if (ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, to_follow)) {
    return ERR_FAILURE;
  }

  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
  pStyle->SetStyleType(ST_PenDesc | ST_BrushDesc | ST_SymbolDesc | ST_AnnoDesc);

  IATOOL_APPEND_MSG(GT_MSG_SET_INPUT_POINT_TYPE);
  IATOOL_APPEND_MSG(GT_MSG_GET_INPUT_POINT_TYPE);

  RegisterMessage();

  return ERR_NONE;
}

int InputPointTool::AuxDraw() { return ERR_NONE; }

int InputPointTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.source_window != m_hWnd) return ERR_NONE;

  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
    } break;
    case GT_MSG_SET_INPUT_POINT_TYPE: {
      m_appendType = *(ushort *)param.wparam;
    } break;
    case GT_MSG_GET_INPUT_ANNO_ANGLE: {
      *(float *)param.wparam = m_fAngle;
    } break;
    case GT_MSG_GET_INPUT_POINT_TYPE: {
      *(ushort *)param.wparam = m_appendType;
    } break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::LButtonDown(uint nFlags, lPoint point) {
  SetOperDone(false);

  POINT pnt;
  GetCursorPos(&pnt);
  ScreenToClient(m_hWnd, &pnt);

  lPoint lpnt(pnt.x, pnt.y);

  switch (m_appendType) {
    case PT_ChildImage:
      AppendChildImage(typeLButtonDown, point);
      break;
    case PT_Text:
      AppendText(typeLButtonDown, point);
      break;
    case PT_DOT:
      AppendDot(typeLButtonDown, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::MouseMove(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case PT_ChildImage:
      AppendChildImage(typeMouseMove, point);
      break;
    case PT_Text:
      AppendText(typeMouseMove, point);
      break;
    case PT_DOT:
      AppendDot(typeMouseMove, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::LButtonUp(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case PT_ChildImage:
      AppendChildImage(typeLButtonUp, point);
      break;
    case PT_Text:
      AppendText(typeLButtonUp, point);
      break;
    case PT_DOT:
      AppendDot(typeLButtonUp, point);
      break;
    default:
      break;
  }
  return ERR_NONE;
}

int InputPointTool::RButtonDown(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case PT_ChildImage:
      AppendChildImage(typeRButtonDown, point);
      break;
    case PT_Text:
      AppendText(typeRButtonDown, point);
      break;
    case PT_DOT:
      AppendDot(typeRButtonDown, point);
      break;
    default:
      break;
  }

  if (IsOperDone()) {
    if (m_bDelay)
      m_bDelay = false;
    else
      SetEnableContexMenu(true);
  }
  return ERR_NONE;
}

int InputPointTool::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void InputPointTool::AppendChildImage(short mouse_status, lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown: {
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      float x, y;
      m_pRenderDevice->DPToLP(m_pntOrigin.x, m_pntOrigin.y, x, y);
      m_pGeom = new Point(x, y);

      SetOperDone(false);
      SetEnableContexMenu(false);
      EndAppendPoint();
    } break;
  }
}

void InputPointTool::AppendText(short mouse_status, lPoint point) {
  static fPoint fpts[2];

  switch (mouse_status) {
    case typeLButtonDown: {
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;
      m_bIsDrag = 1;

      SetOperDone(false);
      SetEnableContexMenu(false);
    }

    break;
    case typeLButtonUp:
      if (m_bIsDrag) {
        m_bIsDrag = 0;
        m_pntCur = point;

        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (ERR_NONE == m_pRenderDevice->BeginRender(
                                MRD_BL_QUICK, false, pStyle, R2_NOTXORPEN)) {
          fpts[0].x = m_pntOrigin.x;
          fpts[0].y = m_pntOrigin.y;
          fpts[1].x = m_pntCur.x;
          fpts[1].y = m_pntCur.y;

          m_pRenderDevice->DrawLine(fpts, 2, true);

          m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }

        float x, y;
        m_fAngle = 0;
        m_pRenderDevice->DPToLP(m_pntOrigin.x, m_pntOrigin.y, x, y);
        m_pGeom = new Point(x, y);
        if (m_pntOrigin != m_pntCur) {
          m_fAngle = atanf(float(m_pntCur.y - m_pntOrigin.y) /
                           float(m_pntCur.x - m_pntOrigin.x));
        }

        EndAppendPoint();
      }
      break;
    case typeMouseMove:
      m_pntPrev = m_pntCur;
      m_pntCur = point;
      if (m_bIsDrag) {
        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (ERR_NONE == m_pRenderDevice->BeginRender(
                                MRD_BL_QUICK, false, pStyle, R2_NOTXORPEN)) {
          fpts[0].x = m_pntOrigin.x;
          fpts[0].y = m_pntOrigin.y;

          fpts[1].x = m_pntPrev.x;
          fpts[1].y = m_pntPrev.y;
          m_pRenderDevice->DrawLine(fpts, 2, true);

          fpts[1].x = m_pntCur.x;
          fpts[1].y = m_pntCur.y;
          m_pRenderDevice->DrawLine(fpts, 2, true);

          m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }
      }
      break;
    default:
      break;
  }
}

void InputPointTool::AppendDot(short mouse_status, core::lPoint point) {
  switch (mouse_status) {
    case typeLButtonDown: {
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      float x, y;
      m_pRenderDevice->DPToLP(m_pntOrigin.x, m_pntOrigin.y, x, y);
      m_pGeom = new Point(x, y);
      SetOperDone(false);
      SetEnableContexMenu(false);
      EndAppendPoint();
    } break;
  }
}

void InputPointTool::EndAppendPoint() {
  //
  m_pRenderDevice->Refresh();

  ushort uRetType = GT_MSG_RET_INPUT_POINT;
  ListenerMessage param;

  param.source_window = m_hWnd;
  param.wparam = WPARAM(m_pGeom);
  param.lparam = LPARAM(&uRetType);

  EndIA(GT_MSG_RET_DELEGATE, param);

  SAFE_DELETE(m_pGeom);

  SetOperDone(true);
}
}  // namespace _GroupTool