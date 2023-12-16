#include "gt_inputlinetool.h"

#include <math.h>

#include "bl_api.h"
#include "bl_stylemanager.h"
#include "geo_api.h"
#include "geo_geometry.h"
#include "gis_feature.h"
#include "gis_map.h"
#include "gis_sde.h"
#include "gt_defs.h"
#include "resource.h "
#include "smt_api.h"
#include "sys_sysmanager.h"


using namespace _Rd;
using namespace core;
using namespace _GIS;
using namespace core;
using namespace _Sys;

const string CST_STR_INPUTLINE_TOOL_NAME = "ÊäÈëÏß";

namespace _GroupTool {
InputLineTool::InputLineTool()
    : m_pGeom(NULL), m_bIsDrag(false), m_bDelay(false), m_appendType(-1) {
  SetName(CST_STR_INPUTLINE_TOOL_NAME.c_str());
}

InputLineTool::~InputLineTool() {
  SMT_SAFE_DELETE(m_pGeom);

  UnRegisterMessage();
}

int InputLineTool::Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap,
                        HWND hWnd, pfnToolCallBack pfnCallBack,
                        void *pToFollow) {
  if (SMT_ERR_NONE != BaseTool::Init(pMrdRenderDevice, pOperMap, hWnd,
                                     pfnCallBack, pToFollow)) {
    return SMT_ERR_FAILURE;
  }

  StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
  Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
  if (pStyle) {
    pStyle->SetStyleType(ST_PenDesc);
  }

  SMT_IATOOL_APPEND_MSG(GT_MSG_SET_INPUT_LINE_TYPE);
  SMT_IATOOL_APPEND_MSG(GT_MSG_GET_INPUT_LINE_TYPE);

  RegisterMessage();

  return SMT_ERR_NONE;
}

int InputLineTool::AuxDraw() { return SMT_ERR_NONE; }

int InputLineTool::Notify(long nMessage, ListenerMessage &param) {
  if (param.hSrcWnd != m_hWnd) return SMT_ERR_NONE;

  switch (nMessage) {
    case GT_MSG_DEFAULT_PROCESS: {
    } break;
    case GT_MSG_SET_INPUT_LINE_TYPE: {
      m_appendType = *(ushort *)param.wParam;
    } break;
    case GT_MSG_GET_INPUT_LINE_TYPE: {
      *(ushort *)param.wParam = m_appendType;
    } break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputLineTool::LButtonDown(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case LT_Arc:
      AppendArc(typeLButtonDown, point);
      break;
    case LT_Rect:
      AppendRect(typeLButtonDown, point);
      break;
    case LT_LineString:
      AppendLineString(typeLButtonDown, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(typeLButtonDown, point);
      break;
    case LT_Spline_Lag:
    case LT_Spline_Bzer:
    case LT_Spline_B:
    case LT_Spline_3:
      AppendSpline(typeLButtonDown, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputLineTool::MouseMove(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case LT_Arc:
      AppendArc(typeMouseMove, point);
      break;
    case LT_Rect:
      AppendRect(typeMouseMove, point);
      break;
    case LT_LineString:
      AppendLineString(typeMouseMove, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(typeMouseMove, point);
      break;
    case LT_Spline_Lag:
    case LT_Spline_Bzer:
    case LT_Spline_B:
    case LT_Spline_3:
      AppendSpline(typeMouseMove, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputLineTool::LButtonUp(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case LT_Arc:
      AppendArc(typeLButtonUp, point);
      break;
    case LT_Rect:
      AppendRect(typeLButtonUp, point);
      break;
    case LT_LineString:
      AppendLineString(typeLButtonUp, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(typeLButtonUp, point);
      break;
    case LT_Spline_Lag:
    case LT_Spline_Bzer:
    case LT_Spline_B:
    case LT_Spline_3:
      AppendSpline(typeLButtonUp, point);
      break;
    default:
      break;
  }
  return SMT_ERR_NONE;
}

int InputLineTool::RButtonDown(uint nFlags, lPoint point) {
  switch (m_appendType) {
    case LT_Arc:
      AppendArc(typeRButtonDown, point);
      break;
    case LT_Rect:
      AppendRect(typeRButtonDown, point);
      break;
    case LT_LineString:
      AppendLineString(typeRButtonDown, point);
      break;
    case LT_LinearRing:
      AppendLinearRing(typeRButtonDown, point);
      break;
    case LT_Spline_Lag:
    case LT_Spline_Bzer:
    case LT_Spline_B:
    case LT_Spline_3:
      AppendSpline(typeRButtonDown, point);
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

int InputLineTool::MouseWeel(uint nFlags, short zDelta, lPoint point) {
  return SMT_ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
void InputLineTool::AppendLineString(uint mouse_status, lPoint point) {
  float x1, y1;
  static fPoint fpts[2];

  switch (mouse_status) {
    case typeLButtonDown:

      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      if (!m_bIsDrag) {
        LineString *pLineString = new LineString();

        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pLineString->AddPoint(x1, y1);

        m_pGeom = pLineString;
        m_bIsDrag = true;

        SetOperDone(false);
        SetEnableContexMenu(false);
      } else {
        LineString *pLineString = (LineString *)m_pGeom;

        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pLineString->AddPoint(x1, y1);
      }

      break;
    case typeRButtonDown:
      if (m_bIsDrag) {
        m_bIsDrag = 0;
        m_pntCur = point;

        LineString *pLineString = (LineString *)m_pGeom;
        EndAppendLine();

        m_bDelay = true;
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

void InputLineTool::AppendArc(uint mouse_status, lPoint point) {
  float x1, y1;
  static fPoint fpts[2];

  switch (mouse_status) {
    case typeLButtonDown: {
      if (!m_bIsDrag) {
        m_pntOrigin = point;
        m_pntPrev = point;
        m_bIsDrag = true;
        m_nStep = 1;

        Arc *pArc = new Arc();
        m_pGeom = pArc;

        SetOperDone(false);
        SetEnableContexMenu(false);
      }

      m_pntPrev = m_pntCur;
      m_pntCur = point;
      m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);

      Arc *pArc = (Arc *)m_pGeom;
      if (m_nStep == 1)
        pArc->SetCenterPoint(RawPoint(x1, y1));
      else if (m_nStep == 2) {
        Point oCtPoint;

        pArc->GetCenterPoint(&oCtPoint);
        float r = Distance(oCtPoint.GetX(), oCtPoint.GetY(), x1, y1);
        pArc->SetStartPoint(RawPoint(x1, y1));
        pArc->SetRadius(r);
      } else if (m_nStep == 3) {
        // pArc->SetEndPoint(RawPoint(x1,y1));
        m_bIsDrag = false;
        m_nStep = 1;
        EndAppendLine();

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
        Arc *pArc = (Arc *)m_pGeom;
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

void InputLineTool::AppendRect(uint mouse_status, lPoint point) {
  float x1, y1;
  static lPoint lpts[2];

  switch (mouse_status) {
    case typeLButtonDown: {
      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;
      m_bIsDrag = true;

      LinearRing *pLineaRing = new LinearRing();
      m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
      pLineaRing->AddPoint(x1, y1);
      m_pGeom = pLineaRing;
      SetOperDone(false);
      SetEnableContexMenu(false);
    }

    break;
    case typeLButtonUp:
      if (m_bIsDrag) {
        m_bIsDrag = false;
        m_pntCur = point;
        m_bIsDrag = 0;

        EndAppendLine();

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

void InputLineTool::AppendLinearRing(uint mouse_status, lPoint point) {
  float x1, y1;
  long x, y;
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
        m_pntCur = point;
        if (m_nStep < 3)
          MessageBeep(0xFFFFFFFF);
        else {
          m_bIsDrag = 0;
          m_bDelay = true;
          m_nStep = 0;
          fpts[0].x = m_pntCur.x;
          fpts[0].y = m_pntCur.y;

          LinearRing *pLinearRing = (LinearRing *)m_pGeom;
          m_pRenderDevice->LPToDP(pLinearRing->GetX(0), pLinearRing->GetY(0), x,
                                  y);

          fpts[1].x = x;
          fpts[1].y = y;

          StyleManager *pStyleMgr = StyleManager::GetSingletonPtr();
          Style *pStyle = pStyleMgr->GetStyle(m_szStyleName);
          if (SMT_ERR_NONE == m_pRenderDevice->BeginRender(
                                  MRD_BL_QUICK, false, pStyle, R2_NOTXORPEN)) {
            m_pRenderDevice->DrawLine(fpts, 2, true);

            m_pRenderDevice->EndRender(MRD_BL_QUICK);

            m_pRenderDevice->RenderMap();
          }

          pLinearRing->CloseRings();
          EndAppendLine();

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

void InputLineTool::AppendSpline(uint mouse_status, lPoint point) {
  float x1, y1;
  static lPoint lpts[2];

  switch (mouse_status) {
    case typeLButtonDown:

      m_pntOrigin = point;
      m_pntPrev = point;
      m_pntCur = point;

      if (!m_bIsDrag) {
        Spline *pSpline = new Spline();

        switch (m_appendType) {
          case LT_Spline_Lag:
            pSpline->SetAnalyticType(CT_LAGSPLINE);
            break;
          case LT_Spline_Bzer:
            pSpline->SetAnalyticType(CT_BZERSPLINE);
            break;
          case LT_Spline_B:
            pSpline->SetAnalyticType(CT_BSPLINE);
            break;
          case LT_Spline_3:
            pSpline->SetAnalyticType(CT_3SPLINE);
            break;
        }

        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pSpline->AddPoint(x1, y1);
        m_pGeom = pSpline;
        m_bIsDrag = true;
        SetOperDone(false);
        SetEnableContexMenu(false);
      } else {
        Spline *pSpline = (Spline *)m_pGeom;
        m_pRenderDevice->DPToLP(point.x, point.y, x1, y1);
        pSpline->AddPoint(x1, y1);
      }

      break;
    case typeRButtonDown:
      if (m_bIsDrag) {
        m_bIsDrag = 0;
        m_pntCur = point;
        Spline *pSpline = (Spline *)m_pGeom;
        pSpline->CalcAnalyticPoints();
        EndAppendLine();

        if (SMT_ERR_NONE ==
            m_pRenderDevice->BeginRender(MRD_BL_QUICK, true, false))
          m_pRenderDevice->EndRender(MRD_BL_QUICK);

        m_bDelay = true;
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
          SysManager *pSysMgr = SysManager::GetSingletonPtr();
          SystemOptions sysPra = pSysMgr->GetSysPra();

          int r = sysPra.point_raduis;
          Spline *pSpline = (Spline *)m_pGeom;
          long lX = 0, lY = 0;
          for (int i = 0; i < pSpline->GetNumPoints(); i++) {
            m_pRenderDevice->LPToDP(pSpline->GetX(i), pSpline->GetY(i), lX, lY);
            m_pRenderDevice->DrawEllipse(lX - r, lY - r, lX + r, lY + r, true);
          }

          m_pRenderDevice->EndRender(MRD_BL_QUICK);
        }
      }
      break;
    default:
      break;
  }
}

void InputLineTool::EndAppendLine(void) {
  m_pRenderDevice->Refresh();

  ushort uRetType = GT_MSG_RET_INPUT_LINE;
  ListenerMessage param;

  param.hSrcWnd = m_hWnd;
  param.wParam = WPARAM(m_pGeom);
  param.lParam = LPARAM(&uRetType);

  EndIA(GT_MSG_RET_DELEGATE, param);

  SMT_SAFE_DELETE(m_pGeom);

  SetOperDone(true);
}
}  // namespace _GroupTool