#include "gt_inputregiontool.h"

#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "geo_api.h"
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

const string CST_STR_INPUTREGION_TOOL_NAME = "ÊäÈëµã";

namespace _GroupTool {
InputRegionTool::InputRegionTool()
    : m_pGeom(NULL),
      m_bIsDrag(false),
      m_bDelay(false),
      m_appendType(-1),
      m_nStep(0) {
  SetName(CST_STR_INPUTREGION_TOOL_NAME.c_str());
}

InputRegionTool::~InputRegionTool() {
  SMT_SAFE_DELETE(m_pGeom);

  UnRegisterMessage();
}

int InputRegionTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
                          HWND hWnd, pfnToolCallBack pfnCallBack,
                          void *pToFollow) {
  if (SMT_ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, pToFollow)) {
    return SMT_ERR_FAILURE;
  }

  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);

  pStyle->SetStyleType(ST_PenDesc | ST_BrushDesc);

  SMT_IATOOL_APPEND_MSG(GT_MSG_SET_INPUT_REGION_TYPE);
  SMT_IATOOL_APPEND_MSG(GT_MSG_GET_INPUT_REGION_TYPE);

  RegisterMessage();

  return SMT_ERR_NONE;
}

int InputRegionTool::AuxDraw() { return SMT_ERR_NONE; }

int InputRegionTool::Notify(long nMessage, ListenerMessage &param) {
  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
      ;
    } break;
    case GT_MSG_SET_INPUT_REGION_TYPE: {
      m_appendType = *(ushort *)param.wParam;
    } break;
    case GT_MSG_GET_INPUT_REGION_TYPE: {
      *(ushort *)param.wParam = m_appendType;
    } break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputRegionTool::LButtonDown(uint nFlags, lPoint point) {
  SetOperDone(false);

  switch (m_appendType) {
    case RT_Fan:
      AppendFan(typeLButtonDown, point);
      break;
    case RT_Rect:
      AppendRect(typeLButtonDown, point);
      break;
    case RT_Polygon:
      AppendPolygon(typeLButtonDown, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputRegionTool::MouseMove(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case RT_Fan:
      AppendFan(typeMouseMove, point);
      break;
    case RT_Rect:
      AppendRect(typeMouseMove, point);
      break;
    case RT_Polygon:
      AppendPolygon(typeMouseMove, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputRegionTool::LButtonUp(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case RT_Fan:
      AppendFan(typeLButtonUp, point);
      break;
    case RT_Rect:
      AppendRect(typeLButtonUp, point);
      break;
    case RT_Polygon:
      AppendPolygon(typeLButtonUp, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputRegionTool::RButtonDown(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case RT_Fan:
      AppendFan(typeRButtonDown, point);
      break;
    case RT_Rect:
      AppendRect(typeRButtonDown, point);
      break;
    case RT_Polygon:
      AppendPolygon(typeRButtonDown, point);
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

  return SMT_ERR_NONE;
}

int InputRegionTool::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void InputRegionTool::AppendFan(uint mouse_status, lPoint point) {
  float x1, y1;
  static fPoint fpts[2];

  switch (mouse_status) {
    case typeLButtonDown: {
      if (!m_bIsDrag) {
        m_pntOrigin = point;
        m_pntPrev = point;
        m_bIsDrag = true;
        m_nStep = 1;
        Fan *pFan = new Fan();
        m_pGeom = pFan;
        pFan->SetArcDirectly(new Arc());

        SetOperDone(false);
        SetEnableContexMenu(false);
      }

      m_pntPrev = m_pntCur;
      m_pntCur = point;

      m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
      Arc *pArc = ((Fan *)m_pGeom)->GetArc();
      if (m_nStep == 1)
        pArc->SetCenterPoint(RawPoint(x1, y1));
      else if (m_nStep == 2) {
        Point oCtPoint;

        pArc->GetCenterPoint(&oCtPoint);
        pArc->SetStartPoint(RawPoint(x1, y1));
        pArc->SetRadius(Distance(oCtPoint.GetX(), oCtPoint.GetY(), x1, y1));
      } else if (m_nStep == 3) {
        m_bIsDrag = false;
        m_nStep = 1;

        EndAppendRegion();

        if (SMT_ERR_NONE ==
            m_pRenderDevice->BeginRender(MRD_BL_QUICK, true, false))
          m_pRenderDevice->EndRender(MRD_BL_QUICK);
      }

      m_nStep++;
    } break;
    case typeMouseMove:

      m_pntPrev = m_pntCur;
      m_pntCur = point;

      if (m_bIsDrag) {
        Arc *pArc = ((Fan *)m_pGeom)->GetArc();
        if (m_nStep == 3) {
          Point oCtPoint, oStPoint;
          pArc->GetCenterPoint(&oCtPoint);
          pArc->StartPoint(&oStPoint);
          //
          m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
          float R = pArc->GetRadius();
          float r = Distance(oCtPoint.GetX(), oCtPoint.GetY(), x1, y1);

          //
          x1 = (x1 - oCtPoint.GetX()) * R / r + oCtPoint.GetX();
          y1 = (y1 - oCtPoint.GetY()) * R / r + oCtPoint.GetY();

          //
          pArc->SetEndPoint(RawPoint(x1, y1));
          m_pRenderDevice->Refresh();
        }

        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (SMT_ERR_NONE == m_pRenderDevice->BeginRender(
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

void InputRegionTool::AppendRect(uint mouse_status, lPoint point) {
  float x1, y1;
  static lPoint lpts[2];

  switch (mouse_status) {
    case typeLButtonDown: {
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;
      m_bIsDrag = true;

      SetOperDone(false);
      SetEnableContexMenu(false);

      LinearRing *pLineaRing = new LinearRing();
      m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
      pLineaRing->AddPoint(x1, y1);
      m_pGeom = pLineaRing;
    } break;
    case typeLButtonUp:
      if (m_bIsDrag) {
        m_bIsDrag = false;
        m_pntCur = point;

        LinearRing *pLinearRing = (LinearRing *)m_pGeom;
        Polygon *pPolygon = new Polygon();
        pPolygon->AddRingDirectly(pLinearRing);
        m_pGeom = pPolygon;

        EndAppendRegion();

        if (SMT_ERR_NONE ==
            m_pRenderDevice->BeginRender(MRD_BL_QUICK, true, false))
          m_pRenderDevice->EndRender(MRD_BL_QUICK);
      }
      break;
    case typeMouseMove:

      m_pntPrev = m_pntCur;
      m_pntCur = point;

      if (m_bIsDrag) {
        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (SMT_ERR_NONE == m_pRenderDevice->BeginRender(
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

        float x1, y1, x2, y2;
        LinearRing *pLinearRing = (LinearRing *)m_pGeom;
        m_pRenderDevice->DPToLP(m_pntOrigin.x, m_pntOrigin.y, x1, y1);
        m_pRenderDevice->DPToLP(m_pntCur.x, m_pntCur.y, x2, y2);
        pLinearRing->SetPoint(1, x2, y1);
        pLinearRing->SetPoint(2, x2, y2);
        pLinearRing->SetPoint(3, x1, y2);
        pLinearRing->CloseRings();
      }
      break;
    default:
      break;
  }
}

void InputRegionTool::AppendPolygon(uint mouse_status, lPoint point) {
  float x1, y1;
  static fPoint fpts[2];

  switch (mouse_status) {
    case typeLButtonDown:

      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      if (!m_bIsDrag) {
        LinearRing *pLinearRing = new LinearRing();
        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pLinearRing->AddPoint(x1, y1);
        m_pGeom = pLinearRing;
        m_bIsDrag = true;
        m_nStep = 0;
        SetOperDone(false);
        SetEnableContexMenu(false);
      } else {
        LinearRing *pLinearRing = (LinearRing *)m_pGeom;
        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pLinearRing->AddPoint(x1, y1);
      }

      m_nStep++;

      break;
    case typeRButtonDown:
      if (m_bIsDrag) {
        if (m_nStep < 3)
          MessageBeep(0xFFFFFFFF);
        else {
          m_bIsDrag = 0;
          m_pntCur = point;
          m_bDelay = true;
          m_nStep = 0;

          LinearRing *pLinearRing = (LinearRing *)m_pGeom;
          pLinearRing->CloseRings();

          Polygon *pPolygon = new Polygon();
          pPolygon->AddRingDirectly(pLinearRing);
          m_pGeom = pPolygon;
          EndAppendRegion();

          if (SMT_ERR_NONE ==
              m_pRenderDevice->BeginRender(MRD_BL_QUICK, true, false))
            m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }
      }
      break;
    case typeMouseMove:
      m_pntPrev = m_pntCur;
      m_pntCur = point;

      if (m_bIsDrag) {
        StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
        Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
        if (SMT_ERR_NONE == m_pRenderDevice->BeginRender(
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

void InputRegionTool::EndAppendRegion() {
  m_pRenderDevice->Refresh();

  ushort uRetType = GT_MSG_RET_INPUT_REGION;
  ListenerMessage param;

  param.hSrcWnd = m_hWnd;
  param.wParam = WPARAM(m_pGeom);
  param.lParam = LPARAM(&uRetType);

  EndIA(GT_MSG_RET_DELEGATE, param);

  SMT_SAFE_DELETE(m_pGeom);
  SetOperDone(true);
}
}  // namespace _GroupTool