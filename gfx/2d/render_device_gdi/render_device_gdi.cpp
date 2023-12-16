#include "gfx/2d/render_device_gdi/render_device_gdi.h"

#include <math.h>

namespace gfx2d {
const float C_fDELAY = 0.25;

int CreateRenderDevice(HINSTANCE hInst, LPRENDERDEVICE &pMrdDevice) {
  if (!pMrdDevice) {
    pMrdDevice = new RenderDeviceGDI(hInst);
    return ERR_NONE;
  }

  return ERR_FAILURE;
}

int DestroyRenderDevice(LPRENDERDEVICE &pMrdDevice) {
  if (!pMrdDevice) return ERR_FAILURE;

  SAFE_DELETE(pMrdDevice);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
RenderDeviceGDI::RenderDeviceGDI(HINSTANCE hInst)
    : RenderDevice(hInst),
      m_hFont(NULL),
      m_hPen(NULL),
      m_hBrush(NULL),
      m_hIcon(NULL),
      m_hOldFont(NULL),
      m_hOldPen(NULL),
      m_hOldBrush(NULL),
      m_hCurDC(NULL),
      m_fAnnoAngle(0.),
      m_nFeatureType(FeatureType::FtUnknown),
      m_llLastRedrawCmdStamp(0),
      m_llLastRedrawStamp(0),
      m_bRedraw(false),
      m_bCurUseStyle(false),
      m_bLockStyle(false) {
  m_rBaseApi = RD_GDI;

  m_curDrawingOrg.x = 0;
  m_curDrawingOrg.y = 0;
  sprintf(m_szAnno, "Smart Gis");
}

RenderDeviceGDI::~RenderDeviceGDI(void) { Release(); }

int RenderDeviceGDI::Init(HWND hWnd, const char *logname) {
  if (hWnd == NULL || logname == NULL) return ERR_INVALID_PARAM;

  m_hWnd = hWnd;

  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->CreateLog(logname);

  if (NULL == pLog) return ERR_FAILURE;

  pLog->LogMessage("Init Gdi SimpleRenderDevice ok!");

  m_strLogName = logname;

  m_smtRenderBuf.SetWnd(m_hWnd);
  m_smtMapRenderBuf.SetWnd(m_hWnd);

  return ERR_NONE;
}

int RenderDeviceGDI::Destroy(void) {
  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetLog(m_strLogName.c_str());
  if (pLog != NULL) pLog->LogMessage("Destroy Gdi SimpleRenderDevice ok!");

  return ERR_NONE;
}

int RenderDeviceGDI::Release(void) {
  LogManager *pLogMgr = LogManager::GetSingletonPtr();
  Log *pLog = pLogMgr->GetLog(m_strLogName.c_str());
  if (pLog != NULL) pLog->LogMessage("Release Gdi SimpleRenderDevice ok!");

  if (m_hFont) {
    DeleteObject(m_hFont);
    m_hFont = NULL;
  }

  if (m_hPen) {
    DeleteObject(m_hPen);
    m_hPen = NULL;
  }

  if (m_hBrush) {
    DeleteObject(m_hBrush);
    m_hBrush = NULL;
  }

  if (m_hIcon) {
    DeleteObject(m_hIcon);
    m_hIcon = NULL;
  }

  return ERR_NONE;
}

int RenderDeviceGDI::Resize(int orgx, int orgy, int cx, int cy) {
  if (cx < 0 || cy < 0) return ERR_FAILURE;

  if (IsEqual(m_Viewport.m_fVOX, orgx, dEPSILON) &&
      IsEqual(m_Viewport.m_fVOY, orgy, dEPSILON) &&
      IsEqual(m_Viewport.m_fVHeight, cy, dEPSILON) &&
      IsEqual(m_Viewport.m_fVWidth, cx, dEPSILON)) {
    return ERR_FAILURE;
  }

  m_Viewport.m_fVOX = orgx;
  m_Viewport.m_fVOY = orgy;
  m_Viewport.m_fVHeight = cy;
  m_Viewport.m_fVWidth = cx;

  m_virViewport1 = m_Viewport;
  m_virViewport2 = m_Viewport;

  float xblc, yblc;
  xblc = m_Viewport.m_fVWidth / m_Windowport.m_fWWidth;
  yblc = m_Viewport.m_fVHeight / m_Windowport.m_fWHeight;

  m_fblc = (xblc > yblc) ? yblc : xblc;

  if (ERR_NONE == m_smtRenderBuf.SetBufSize(m_Viewport.m_fVWidth,
                                                m_Viewport.m_fVHeight) &&
      ERR_NONE == m_smtMapRenderBuf.SetBufSize(m_Viewport.m_fVWidth,
                                                   m_Viewport.m_fVHeight)) {
    if (ERR_NONE ==
            m_smtRenderBuf.SwapBuf(m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                                   m_Viewport.m_fVWidth, m_Viewport.m_fVHeight,
                                   m_Viewport.m_fVOX, m_Viewport.m_fVOY) &&
        ERR_NONE == m_smtMapRenderBuf.SwapBuf(
                            m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                            m_Viewport.m_fVWidth, m_Viewport.m_fVHeight,
                            m_Viewport.m_fVOX, m_Viewport.m_fVOY)) {
      return ERR_NONE;
    }
  }

  return ERR_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::LPToDP(float x, float y, LONG &X, LONG &Y) const {
  if (IsEqual(m_Windowport.m_fWWidth, 0, dEPSILON) &&
      IsEqual(m_Windowport.m_fWHeight, 0, dEPSILON) &&
      IsEqual(m_Viewport.m_fVWidth, 0, dEPSILON) &&
      IsEqual(m_Viewport.m_fVHeight, 0, dEPSILON)) {
    X = x;
    Y = y;

    return ERR_FAILURE;
  }

  X = LONG(m_Viewport.m_fVOX + (x - m_Windowport.m_fWOX) * m_fblc + 0.5);
  Y = LONG(m_Viewport.m_fVOY + (y - m_Windowport.m_fWOY) * m_fblc + 0.5);

  Y = m_Viewport.m_fVHeight - Y;

  return ERR_NONE;
}

int RenderDeviceGDI::DPToLP(LONG X, LONG Y, float &x, float &y) const {
  if (IsEqual(m_Windowport.m_fWWidth, 0, dEPSILON) &&
      IsEqual(m_Windowport.m_fWHeight, 0, dEPSILON) &&
      IsEqual(m_Viewport.m_fVWidth, 0, dEPSILON) &&
      IsEqual(m_Viewport.m_fVHeight, 0, dEPSILON)) {
    x = X;
    y = Y;

    return ERR_FAILURE;
  }

  Y = m_Viewport.m_fVHeight - Y;

  x = (X - m_Viewport.m_fVOX) / m_fblc + m_Windowport.m_fWOX;
  y = (Y - m_Viewport.m_fVOY) / m_fblc + m_Windowport.m_fWOY;

  return ERR_NONE;
}

int RenderDeviceGDI::LRectToDRect(const fRect &frect, lRect &lrect) const {
  LPToDP(frect.lb.x, frect.lb.y, lrect.lb.x, lrect.lb.y);
  LPToDP(frect.rt.x, frect.rt.y, lrect.rt.x, lrect.rt.y);

  return ERR_NONE;
}

int RenderDeviceGDI::DRectToLRect(const lRect &lrect, fRect &frect) const {
  DPToLP(lrect.lb.x, lrect.lb.y, frect.lb.x, frect.lb.y);
  DPToLP(lrect.rt.x, lrect.rt.y, frect.rt.x, frect.rt.y);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::Lock() {
  m_cslock.Lock();
  return ERR_NONE;
}

int RenderDeviceGDI::Unlock() {
  m_cslock.Unlock();
  return ERR_NONE;
}

int RenderDeviceGDI::Refresh() {
  //
  int invalidatex1, invalidatey1, invalidatew1, invalidateh1;
  int invalidatex2, invalidatey2, invalidatew2, invalidateh2;
  if (m_curDrawingOrg.x >= 0) {
    if (m_curDrawingOrg.y >= 0) {
      invalidatex1 = invalidatey1 = 0;
      invalidatew1 = m_Viewport.m_fVWidth, invalidateh1 = m_curDrawingOrg.y;
      invalidatex2 = 0, invalidatey2 = m_curDrawingOrg.y;
      invalidatew2 = m_curDrawingOrg.x,
      invalidateh2 = m_Viewport.m_fVHeight - m_curDrawingOrg.y;
    } else {
      invalidatex1 = invalidatey1 = 0;
      invalidatew1 = m_curDrawingOrg.x,
      invalidateh1 = m_Viewport.m_fVHeight + m_curDrawingOrg.y;
      invalidatex2 = 0,
      invalidatey2 = m_Viewport.m_fVHeight + m_curDrawingOrg.y;
      invalidatew2 = m_Viewport.m_fVWidth, invalidateh2 = -m_curDrawingOrg.y;
    }
  } else {
    if (m_curDrawingOrg.y >= 0) {
      invalidatex1 = invalidatey1 = 0;
      invalidatew1 = m_Viewport.m_fVWidth, invalidateh1 = m_curDrawingOrg.y;
      invalidatex2 = m_Viewport.m_fVWidth + m_curDrawingOrg.x,
      invalidatey2 = m_curDrawingOrg.y;
      invalidatew2 = -m_curDrawingOrg.x,
      invalidateh2 = m_Viewport.m_fVHeight - m_curDrawingOrg.y;
    } else {
      invalidatex1 = m_Viewport.m_fVWidth + m_curDrawingOrg.x, invalidatey1 = 0;
      invalidatew1 = -m_curDrawingOrg.x,
      invalidateh1 = m_Viewport.m_fVHeight + m_curDrawingOrg.y;
      invalidatex2 = 0,
      invalidatey2 = m_Viewport.m_fVHeight + m_curDrawingOrg.y;
      invalidatew2 = m_Viewport.m_fVWidth, invalidateh2 = -m_curDrawingOrg.y;
    }
  }

  HDC hDC = GetDC(m_hWnd);
  ClearRect(hDC, invalidatex1, invalidatey1, invalidatew1,
            invalidateh1 /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);
  ClearRect(hDC, invalidatex2, invalidatey2, invalidatew2,
            invalidateh2 /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

  m_smtRenderBuf.ClearBuf(
      m_Viewport.m_fVOX, m_Viewport.m_fVOY, m_Viewport.m_fVWidth,
      m_Viewport.m_fVWidth /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

  m_smtMapRenderBuf.SwapBuf(m_smtRenderBuf, m_virViewport1.m_fVOX,
                            m_virViewport1.m_fVOY, m_virViewport1.m_fVWidth,
                            m_virViewport1.m_fVHeight, m_virViewport2.m_fVOX,
                            m_virViewport2.m_fVOY, m_virViewport2.m_fVWidth,
                            m_virViewport2.m_fVHeight, BLT_TRANSPARENT,
                            SRCCOPY /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

  m_smtRenderBuf.SwapBuf(m_curDrawingOrg.x, m_curDrawingOrg.y,
                         m_Viewport.m_fVWidth, m_Viewport.m_fVHeight,
                         m_Viewport.m_fVOX, m_Viewport.m_fVOY);

  ::ReleaseDC(m_hWnd, hDC);

  RECT rt;
  GetClientRect(m_hWnd, &rt);
  InvalidateRect(m_hWnd, &rt, true);

  return ERR_NONE;
}

int RenderDeviceGDI::Refresh(const Map *pMap, fRect frect) {
  lRect lrect;
  LRectToDRect(frect, lrect);
  RefreshDirectly(pMap, lrect);

  return ERR_NONE;
}

int RenderDeviceGDI::RefreshDirectly(const Map *pMap, lRect rect,
                                     bool bRealTime) {
  if (pMap) {
    return RenderMap(pMap, m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                     m_Viewport.m_fVWidth, m_Viewport.m_fVHeight);
  }

  return ERR_INVALID_PARAM;
}

int RenderDeviceGDI::ZoomMove(const Map *pMap, fPoint dbfPointOffset,
                              bool bRealTime) {
  m_virViewport1.m_fVOX += dbfPointOffset.x * m_fblc;
  m_virViewport1.m_fVOY -= dbfPointOffset.y * m_fblc;

  m_Windowport.m_fWOX -= dbfPointOffset.x;
  m_Windowport.m_fWOY -= dbfPointOffset.y;

  if (pMap) {
    return RenderMap(pMap, m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                     m_Viewport.m_fVWidth, m_Viewport.m_fVHeight);
  }

  return ERR_INVALID_PARAM;
}

int RenderDeviceGDI::ZoomScale(const Map *pMap, lPoint orgPoint, float fscale,
                               bool bRealTime) {
  if (fscale > 1.) {  //改变m_virViewport1
    m_virViewport1.m_fVHeight /= fscale;
    m_virViewport1.m_fVWidth /= fscale;
    m_virViewport1.m_fVOX =
        orgPoint.x - (orgPoint.x - m_virViewport1.m_fVOX) / fscale;
    m_virViewport1.m_fVOY =
        orgPoint.y - (orgPoint.y - m_virViewport1.m_fVOY) / fscale;
  } else {  //改变m_virViewport2
    m_virViewport2.m_fVHeight *= fscale;
    m_virViewport2.m_fVWidth *= fscale;
    m_virViewport2.m_fVOX =
        orgPoint.x + (m_virViewport2.m_fVOX - orgPoint.x) * fscale;
    m_virViewport2.m_fVOY =
        orgPoint.y + (m_virViewport2.m_fVOY - orgPoint.y) * fscale;
  }

  float x1, y1, x2, y2;
  DPToLP(orgPoint.x, orgPoint.y, x1, y1);

  m_Windowport.m_fWHeight *= fscale;
  m_Windowport.m_fWWidth *= fscale;

  m_fblc /= fscale;

  DPToLP(orgPoint.x, orgPoint.y, x2, y2);

  m_Windowport.m_fWOX -= x2 - x1;
  m_Windowport.m_fWOY -= y2 - y1;

  if (pMap) {
    return RenderMap(pMap, m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                     m_Viewport.m_fVWidth, m_Viewport.m_fVHeight);
  }

  return ERR_INVALID_PARAM;
}

int RenderDeviceGDI::ZoomToRect(const Map *pMap, fRect rect, bool bRealTime) {
  lRect rt;
  LRectToDRect(rect, rt);
  m_virViewport2.m_fVOX = rt.lb.x;
  m_virViewport2.m_fVOY = rt.rt.y;
  m_virViewport2.m_fVHeight = rt.Height();
  m_virViewport2.m_fVWidth = rt.Width();

  m_Windowport.m_fWOX = rect.lb.x;
  m_Windowport.m_fWOY = rect.lb.y;
  m_Windowport.m_fWHeight = rect.Height();
  m_Windowport.m_fWWidth = rect.Width();

  float xblc, yblc;
  xblc = m_Viewport.m_fVWidth / m_Windowport.m_fWWidth;
  yblc = m_Viewport.m_fVHeight / m_Windowport.m_fWHeight;

  m_fblc = (xblc > yblc) ? yblc : xblc;

  if (xblc < yblc) {
    m_virViewport2.m_fVOY += rt.Height() * (1 - yblc / xblc);
    m_Windowport.m_fWHeight = rect.Height() * yblc / xblc;
    m_virViewport2.m_fVHeight = rt.Height() * yblc / xblc;
  } else {
    m_Windowport.m_fWWidth = rect.Width() * xblc / yblc;
    m_virViewport2.m_fVWidth = rt.Width() * xblc / yblc;
  }

  if (pMap) {
    return RenderMap(pMap, m_Viewport.m_fVOX, m_Viewport.m_fVOY,
                     m_Viewport.m_fVWidth, m_Viewport.m_fVHeight);
  }

  return ERR_FAILURE;
}

int RenderDeviceGDI::Timer() {
  if (m_bRedraw) {
    double dbfElapse = 0.;
    LONGLONG llStamp = 0, llPerCount = 0;

    QueryPerformanceFrequency((LARGE_INTEGER *)&llPerCount);
    QueryPerformanceCounter((LARGE_INTEGER *)&llStamp);

    dbfElapse = (llStamp - m_llLastRedrawCmdStamp) / (double)llPerCount;

    if (dbfElapse > C_fDELAY) {  //激活重绘
      m_bRedraw = false;
      //重绘
      m_smtMapRenderBuf.ClearBuf(m_nOX, m_nOY, m_nWidth, m_nHeight,
                                 (COLORREF)::GetSysColor(COLOR_WINDOW));

      HDC hPaintBufferDC = m_smtMapRenderBuf.PrepareDC();

      BeginRender(MRD_BL_MAP);

      if (m_pCurMap != NULL) {
        m_pCurMap->MoveFirst();
        while (!m_pCurMap->IsEnd()) {
          RenderLayer(m_pCurMap->GetLayer(), m_nOp);
          m_pCurMap->MoveNext();
        }
      }

      EndRender(MRD_BL_MAP);

      m_virViewport1 = m_Viewport;
      m_virViewport2 = m_Viewport;
    }
  }

  RECT rt;
  GetClientRect(m_hWnd, &rt);
  InvalidateRect(m_hWnd, &rt, true);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::BeginRender(eRDBufferLayer eMRDBufLyr, bool bClear,
                                 const Style *pStyle, int op) {
  m_bLockStyle = (NULL != pStyle);

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      if (bClear)
        m_smtMapRenderBuf.ClearBuf(
            m_Viewport.m_fVOX, m_Viewport.m_fVOY, m_Viewport.m_fVWidth,
            m_Viewport.m_fVHeight /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

      m_hCurDC = m_smtMapRenderBuf.PrepareDC();
    } break;
    case MRD_BL_DIRECT: {
      if (bClear) {
        RECT rt;
        GetClientRect(m_hWnd, &rt);
        InvalidateRect(m_hWnd, &rt, true);
      }

      m_hCurDC = GetDC(m_hWnd);
    } break;
  }

  if (m_bLockStyle) PrepareForDrawing(pStyle, op);

  return ERR_NONE;
}

int RenderDeviceGDI::EndRender(eRDBufferLayer eMRDBufLyr) {
  if (m_bLockStyle) EndDrawing();

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      m_smtMapRenderBuf.EndDC();
    } break;
    case MRD_BL_DIRECT: {
      ReleaseDC(m_hWnd, m_hCurDC);
    } break;
  }

  m_hCurDC = NULL;
  m_bLockStyle = false;

  return ERR_NONE;
}

int RenderDeviceGDI::PrepareForDrawing(const Style *pStyle, int nDrawMode) {
  ::SetROP2(m_hCurDC, nDrawMode);

  m_bCurUseStyle = (NULL != pStyle);

  if (m_bCurUseStyle) {
    ulong format = pStyle->GetStyleType();
    if (format & ST_PenDesc) {
      PenDesc pen = pStyle->GetPenDesc();

      if (m_hPen) {
        DeleteObject(m_hPen);
        m_hPen = NULL;
      }

      m_hPen = CreatePen(pen.lPenStyle, pen.fPenWidth, pen.lPenColor);
      m_hOldPen = (HPEN)::SelectObject(m_hCurDC, m_hPen);
    }

    if (format & ST_BrushDesc) {
      BrushDesc brush = pStyle->GetBrushDesc();

      if (m_hBrush) {
        DeleteObject(m_hBrush);
        m_hBrush = NULL;
      }

      if (brush.brushTp == BrushDesc::BT_Hatch) {
        m_hBrush = CreateHatchBrush(brush.lBrushStyle, brush.lBrushColor);
      } else
        m_hBrush = CreateSolidBrush(brush.lBrushColor);

      m_hOldBrush = (HBRUSH)::SelectObject(m_hCurDC, m_hBrush);
    }

    if (format & ST_SymbolDesc) {
      SymbolDesc symbol = pStyle->GetSymbolDesc();

      if (m_hIcon) {
        DeleteObject(m_hIcon);
        m_hIcon = NULL;
      }

      m_hIcon =
          LoadIcon(m_hInst, MAKEINTRESOURCE(symbol.lSymbolID + IDI_ICON_A));
    }

    if (format & ST_AnnoDesc) {
      AnnotationDesc anno = pStyle->GetAnnoDesc();

      if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = NULL;
      }

      m_hFont =
          CreateFont(anno.fHeight * m_fblc, anno.fWidth * m_fblc,
                     anno.lEscapement, anno.lOrientation, anno.lWeight,
                     anno.lItalic, anno.lUnderline, anno.lStrikeOut,
                     anno.lCharSet, anno.lOutPrecision, anno.lClipPrecision,
                     anno.lQuality, anno.lPitchAndFamily, anno.szFaceName);

      m_hOldFont = (HFONT)::SelectObject(m_hCurDC, m_hFont);
      SetBkMode(m_hCurDC, TRANSPARENT);
      SetTextColor(m_hCurDC, anno.lAnnoClr);

      // m_fAnnoAngle = anno.fAngle;
    }
  }

  return ERR_NONE;
}

int RenderDeviceGDI::EndDrawing() {
  if (m_bCurUseStyle) {
    ::SelectObject(m_hCurDC, m_hOldBrush);
    ::SelectObject(m_hCurDC, m_hOldPen);
    ::SelectObject(m_hCurDC, m_hOldFont);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderMap(void) {
  m_smtRenderBuf.ClearBuf(
      m_Viewport.m_fVOX, m_Viewport.m_fVOY, m_Viewport.m_fVWidth,
      m_Viewport.m_fVHeight /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

  m_smtMapRenderBuf.SwapBuf(m_smtRenderBuf, m_virViewport1.m_fVOX,
                            m_virViewport1.m_fVOY, m_virViewport1.m_fVWidth,
                            m_virViewport1.m_fVHeight, m_virViewport2.m_fVOX,
                            m_virViewport2.m_fVOY, m_virViewport2.m_fVWidth,
                            m_virViewport2.m_fVHeight, BLT_TRANSPARENT,
                            SRCCOPY /*,(COLORREF)::GetSysColor(COLOR_WINDOW)*/);

  m_smtRenderBuf.SwapBuf(m_curDrawingOrg.x, m_curDrawingOrg.y,
                         m_Viewport.m_fVWidth, m_Viewport.m_fVHeight,
                         m_Viewport.m_fVOX, m_Viewport.m_fVOY);

  return ERR_NONE;
}

int RenderDeviceGDI::RenderMap(const Map *pMap, int x, int y, int w, int h,
                               int op) {
  if (w == 0 || h == 0) return ERR_INVALID_PARAM;

  m_pCurMap = pMap;
  m_nOX = x;
  m_nOY = y;
  m_nWidth = w;
  m_nHeight = h;
  m_nOp = op;

  if (!m_bRedraw) {
    m_bRedraw = true;
    QueryPerformanceCounter((LARGE_INTEGER *)&m_llLastRedrawStamp);
  }

  QueryPerformanceCounter((LARGE_INTEGER *)&m_llLastRedrawCmdStamp);

  return ERR_NONE;
}

int RenderDeviceGDI::RenderMap(const Map *pMap, int op) {
  if (NULL == pMap) return ERR_INVALID_PARAM;

  pMap->MoveFirst();
  while (!pMap->IsEnd()) {
    RenderLayer(pMap->GetLayer(), op);
    pMap->MoveNext();
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderLayer(const Layer *pLayer, int op) {
  if (NULL == pLayer) return ERR_INVALID_PARAM;

  if (pLayer->GetLayerType() == LYR_VECTOR)
    return RenderLayer((VectorLayer *)pLayer, op);
  else if (pLayer->GetLayerType() == LYR_RASTER)
    return RenderLayer((RasterLayer *)pLayer, op);
  else if (pLayer->GetLayerType() == LYR_TITLE)
    return RenderLayer((TileLayer *)pLayer, op);

  return ERR_FAILURE;
}

int RenderDeviceGDI::RenderLayer(const VectorLayer *pLayer, int op) {
  if (NULL == pLayer) return ERR_INVALID_PARAM;

  if (!pLayer->IsVisible()) return ERR_NONE;

  Envelope envLayer;
  pLayer->GetEnvelope(envLayer);
  Envelope envViewp;

  lRect lViewp;
  fRect fViewp;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  RectToEnvelope(envViewp, fViewp);

  if (!envLayer.Intersects(envViewp)) return ERR_NONE;

  pLayer->MoveFirst();
  while (!pLayer->IsEnd()) {
    RenderFeature(pLayer->GetFeature(), op);
    pLayer->MoveNext();
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderLayer(const RasterLayer *pLayer, int op) {
  if (NULL == pLayer) return ERR_INVALID_PARAM;

  if (!pLayer->IsVisible()) return ERR_NONE;

  Envelope envLayer;
  pLayer->GetEnvelope(envLayer);
  Envelope envViewp;

  lRect lViewp;
  fRect fViewp;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  RectToEnvelope(envViewp, fViewp);

  if (!envLayer.Intersects(envViewp)) return ERR_NONE;

  char *pRasterBuf = NULL;
  long lRasterBufSize = 0;
  long lCodeType = -1;
  fRect locRect;

  if (ERR_NONE == pLayer->GetRasterNoClone(pRasterBuf, lRasterBufSize,
                                               locRect, lCodeType)) {
    lRect lrt;
    LRectToDRect(locRect, lrt);

    CxImage tmpImage;
    tmpImage.Decode((BYTE *)pRasterBuf, lRasterBufSize, lCodeType);
    tmpImage.Stretch(m_hCurDC, lrt.lb.x, lrt.rt.y, lrt.Width(), lrt.Height());
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderLayer(const TileLayer *pLayer, int op) {
  if (NULL == pLayer) return ERR_INVALID_PARAM;

  if (!pLayer->IsVisible()) return ERR_NONE;

  Envelope envLayer;
  pLayer->GetEnvelope(envLayer);
  Envelope envViewp;

  lRect lViewp;
  fRect fViewp;
  lRect titleDPRect;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  RectToEnvelope(envViewp, fViewp);

  if (!envLayer.Intersects(envViewp)) return ERR_NONE;

  pLayer->MoveFirst();
  while (!pLayer->IsEnd()) {
    Tile *pTile = pLayer->GetTile();
    if (NULL != pTile && pTile->bVisible) {
      /*	Envelope envTile,envViewp;
              RectToEnvelope(envTile,pTile->rtTileRect);
              LRectToDRect(pTile->rtTileRect,titleDPRect);

              if (!envTile.Intersects(envViewp) ||
                      (titleDPRect.Height() < 2 && titleDPRect.Width() < 2))
                      return ERR_NONE;*/

      CxImage tmpImage;
      tmpImage.Decode((BYTE *)pTile->pTileBuf, pTile->lTileBufSize,
                      pTile->lImageCode);
      tmpImage.Stretch(m_hCurDC, titleDPRect.lb.x, titleDPRect.rt.y,
                       titleDPRect.Width(), titleDPRect.Height());
    }

    pLayer->MoveNext();
  }

  return ERR_NONE;
}

int RenderDeviceGDI::RenderFeature(const Feature *pFeature, int op) {
  if (NULL == pFeature) return ERR_INVALID_PARAM;

  const Style *pStyle = pFeature->GetStyle();
  const Geometry *pGeom = pFeature->GetGeometryRef();
  const Attribute *pAtt = pFeature->GetAttributeRef();

  m_nFeatureType = pFeature->GetFeatureType();

  switch (m_nFeatureType) {
    case FeatureType::FtAnno: {
      const Field *pFld = NULL;

      pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("anno"));
      sprintf(m_szAnno, pFld->GetValueAsString());

      pFld = pAtt->GetFieldPtr(pAtt->GetFieldIndex("angle"));
      m_fAnnoAngle = pFld->GetValueAsDouble();
    } break;
    case FeatureType::FtChildImage: {
    } break;
    case FeatureType::FtSurface: {
      // nDrawMode = R2_MASKPEN;
    } break;
    default:
      break;
  }

  return RenderGeometry(pGeom, pStyle, op);
}

int RenderDeviceGDI::RenderGeometry(const Geometry *pGeom, const Style *pStyle,
                                    int op) {
  if (!pGeom) return ERR_INVALID_PARAM;

  GeometryType type = pGeom->GetGeometryType();

  Envelope envFeature, envViewp;
  pGeom->GetEnvelope(&envFeature);

  lRect lViewp;
  fRect fViewp;
  fRect fenv;
  lRect lenv;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  RectToEnvelope(envViewp, fViewp);

  EnvelopeToRect(fenv, envFeature);
  LRectToDRect(fenv, lenv);

  if (!envFeature.Intersects(envViewp) ||
      (type != GTPoint && lenv.Height() < 2 && lenv.Width() < 2))
    return ERR_NONE;

  ::SaveDC(m_hCurDC);

  if (!m_bLockStyle) PrepareForDrawing(pStyle);

  if (m_rdPra.show_mbr) {
    long lX = 0, lY = 0;

    LPToDP(envFeature.MinX, envFeature.MinY, lX, lY);
    MoveToEx(m_hCurDC, lX, lY, NULL);

    LPToDP(envFeature.MaxX, envFeature.MinY, lX, lY);
    LineTo(m_hCurDC, lX, lY);

    LPToDP(envFeature.MaxX, envFeature.MaxY, lX, lY);
    LineTo(m_hCurDC, lX, lY);

    LPToDP(envFeature.MinX, envFeature.MaxY, lX, lY);
    LineTo(m_hCurDC, lX, lY);

    LPToDP(envFeature.MinX, envFeature.MinY, lX, lY);
    LineTo(m_hCurDC, lX, lY);
  }

  switch (type) {
    case GTPoint:
      DrawPoint(pStyle, (Point *)pGeom);
      break;

    case GTLineString:
      DrawLineString((LineString *)pGeom);
      break;
    case GTArc:
      DrawArc((Arc *)pGeom);
      break;

    case GTPolygon:
      DrawPloygon((Polygon *)pGeom);
      break;

    case GTFan:
      DrawFan((Fan *)pGeom);
      break;

    case GTMultiPoint:
      DrawMultiPoint(pStyle, (MultiPoint *)pGeom);
      break;

    case GTMultiLineString:
      DrawMultiLineString((MultiLineString *)pGeom);
      break;

    case GTMultiPolygon:
      DrawMultiPolygon((MultiPolygon *)pGeom);
      break;

    case GTSpline:
      DrawLineSpline((Spline *)pGeom);
      break;

    case GTLinearRing:
      DrawLinearRing((LinearRing *)pGeom);
      break;

    case GTGrid:
      DrawGrid((Grid *)pGeom);
      break;

    case GTTin:
      DrawTin((Tin *)pGeom);
      break;

    case GTNone:
    case GTUnknown:
    default:
      break;
  }

  if (!m_bLockStyle) EndDrawing();

  ::RestoreDC(m_hCurDC, -1);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawMultiLineString(
    const MultiLineString *pMultiLinestring) {
  int nLines = pMultiLinestring->GetNumGeometries();

  int i = 0;
  while (i < nLines) {
    DrawLineString((LineString *)pMultiLinestring->GetGeometryRef(i));
    i++;
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawMultiPoint(const Style *pStyle,
                                    const MultiPoint *pMultiPoint) {
  int nPoints = pMultiPoint->GetNumGeometries();

  int i = 0;
  while (i < nPoints) {
    DrawPoint(pStyle, (Point *)pMultiPoint->GetGeometryRef(i));
    i++;
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawMultiPolygon(const MultiPolygon *pMultiPolygon) {
  int nPolygons = pMultiPolygon->GetNumGeometries();

  int i = 0;
  while (i < nPolygons) {
    DrawPloygon((Polygon *)pMultiPolygon->GetGeometryRef(i));
    i++;
  }

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawPoint(const Style *pStyle, const Point *pPoint) {
  ulong format = pStyle->GetStyleType();
  if (m_nFeatureType == FeatureType::FtAnno) {
    AnnotationDesc anno = pStyle->GetAnnoDesc();
    return DrawAnno(m_szAnno, m_fAnnoAngle, abs(anno.fHeight), abs(anno.fWidth),
                    abs(anno.fSpace), pPoint);
  } else if (m_nFeatureType == FeatureType::FtChildImage) {
    SymbolDesc symbol = pStyle->GetSymbolDesc();
    return DrawSymbol(m_hIcon, symbol.fSymbolHeight, symbol.fSymbolWidth,
                      pPoint);
  } else if (m_nFeatureType == FeatureType::FtDot) {
    int r = m_rdPra.point_raduis /**m_fblc*/;
    long lX, lY;
    LPToDP(pPoint->GetX(), pPoint->GetY(), lX, lY);
    Ellipse(m_hCurDC, lX - r, lY - r, lX + r, lY + r);

    if (m_rdPra.show_point) {
      // Rectangle(m_hCurDC,lX - r,lY - r,lX + r,lY + r);
      DrawCross(m_hCurDC, lX, lY, r);
    }

    return ERR_NONE;
  }

  return ERR_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawAnno(const char *szAnno, float fangel, float fCHeight,
                              float fCWidth, float fCSpace,
                              const Point *pPoint) {
  if (szAnno == NULL) return ERR_INVALID_PARAM;

  fCHeight *= m_fblc;
  fCWidth *= m_fblc;
  fCSpace *= m_fblc;

  unsigned char c1, c2;
  fPoint pt;
  long x, y;
  char bz[4];
  const char *ls1;
  ls1 = szAnno;

  LPToDP(pPoint->GetX(), pPoint->GetY(), x, y);
  pt.x = x;
  pt.y = y;

  pt.x -= 2 * fCHeight * sin(fangel);
  pt.y -= 2 * fCHeight * cos(fangel);

  int nStrLength = (int)strlen(ls1);
  while (nStrLength > 0) {
    c1 = *ls1;
    c2 = *(ls1 + 1);
    if (c1 > 127 && c2 > 127)  //如果下一个字符是汉字
    {
      strncpy(bz, ls1, 2);
      bz[2] = 0;
      ls1 = ls1 + 2;
      TextOut(m_hCurDC, pt.x, pt.y, (LPCSTR)bz, 2);
      nStrLength -= 2;
      pt.x += (fCWidth * 2 + fCSpace) * cos(fangel);
      pt.y += (fCWidth * 2 + fCSpace) * sin(fangel);
    } else {
      strncpy(bz, ls1, 1);
      bz[1] = 0;
      ls1++;
      TextOut(m_hCurDC, pt.x, pt.y, (LPCSTR)bz, 1);
      nStrLength -= 1;

      pt.x += (fCWidth + fCSpace / 2.) * cos(fangel);
      pt.y += (fCWidth + fCSpace / 2.) * sin(fangel);
    }
  }

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    long lX, lY;
    LPToDP(pPoint->GetX(), pPoint->GetY(), lX, lY);
    // Ellipse(m_hCurDC,lX - r ,lY - r,lX + r ,lY + r);
    // Rectangle(m_hCurDC,lX - r,lY - r,lX + r,lY + r);
    DrawCross(m_hCurDC, lX, lY, r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawSymbol(HICON hIcon, long lHeight, long lWidth,
                                const Point *pPoint) {
  lHeight *= m_fblc;
  lWidth *= m_fblc;

  long lX, lY;
  LPToDP(pPoint->GetX(), pPoint->GetY(), lX, lY);
  //::DrawIcon(m_hCurDC,pt.x-lWidth,pt.y-lHeight,hIcon);
  ::DrawIconEx(m_hCurDC, lX - lWidth / 2, lY + lHeight / 2, hIcon, lWidth,
               lHeight, 0, NULL, DI_NORMAL);
  //::DrawState(m_hCurDC,NULL,NULL,(LPARAM)hIcon,0,pt.x-lWidth/2,pt.y+lHeight/2,lWidth,lHeight,
  // DSS_NORMAL | DST_ICON);

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    long lX, lY;
    LPToDP(pPoint->GetX(), pPoint->GetY(), lX, lY);
    // Ellipse(m_hCurDC,lX - r ,lY - r,lX + r ,lY + r);
    // Rectangle(m_hCurDC,lX - r,lY - r,lX + r,lY + r);
    DrawCross(m_hCurDC, lX, lY, r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLineSpline(const Spline *pSpline) {
  int nPoints = pSpline->GetAnalyticPointCount();
  if (nPoints < 2) return ERR_INVALID_PARAM;

  int i = 0;
  POINT *lpPoint = NULL;

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
    lpPoint = (POINT *)m_bufPool.NewBuf();
  } else
    lpPoint = new POINT[nPoints];
#else
  lpPoint = new POINT[nPoints];
#endif

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    for (int i = 0; i < nPoints; i++) {
      LPToDP(pSpline->GetAnalyticX(i), pSpline->GetAnalyticY(i), lpPoint[i].x,
             lpPoint[i].y);
      DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
    }

    MoveToEx(m_hCurDC, lpPoint[0].x, lpPoint[0].y, NULL);
    PolylineTo(m_hCurDC, lpPoint, nPoints);
  } else {
    for (int i = 0; i < nPoints; i++) {
      LPToDP(pSpline->GetAnalyticX(i), pSpline->GetAnalyticY(i), lpPoint[i].x,
             lpPoint[i].y);
    }

    MoveToEx(m_hCurDC, lpPoint[0].x, lpPoint[0].y, NULL);
    PolylineTo(m_hCurDC, lpPoint, nPoints);
  }

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
  } else
    SAFE_DELETE_A(lpPoint);
#else
  SAFE_DELETE_A(lpPoint);
#endif

  int r = m_rdPra.point_raduis;
  long lX, lY;
  for (int i = 0; i < pSpline->GetNumPoints(); i++) {
    LPToDP(pSpline->GetX(i), pSpline->GetY(i), lX, lY);
    Ellipse(m_hCurDC, lX - r, lY - r, lX + r, lY + r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLineString(const LineString *pLinestring) {
  int nPoints = pLinestring->GetNumPoints();
  if (nPoints < 2) return ERR_INVALID_PARAM;

  int i = 0;
  POINT *lpPoint = NULL;

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
    lpPoint = (POINT *)m_bufPool.NewBuf();
  } else
    lpPoint = new POINT[nPoints];
#else
  lpPoint = new POINT[nPoints];
#endif

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    for (int i = 0; i < nPoints; i++) {
      LPToDP(pLinestring->GetX(i), pLinestring->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
      // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
      // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y -
      // r,lpPoint[i].x + r ,lpPoint[i].y + r);
      DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
    }
  } else {
    for (int i = 0; i < nPoints; i++) {
      LPToDP(pLinestring->GetX(i), pLinestring->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
    }
  }

  MoveToEx(m_hCurDC, lpPoint[0].x, lpPoint[0].y, NULL);
  PolylineTo(m_hCurDC, lpPoint, nPoints);

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
  } else
    SAFE_DELETE_A(lpPoint);
#else
  SAFE_DELETE_A(lpPoint);
#endif

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLinearRing(const LinearRing *pLinearRing) {
  int nPoints = pLinearRing->GetNumPoints();
  if (nPoints < 2) return ERR_INVALID_PARAM;

  int i = 0;
  POINT *lpPoint = NULL;
#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) < m_bufPool.GetPoolSize()) {
    m_bufPool.FreeAllBuf();
    lpPoint = (POINT *)m_bufPool.NewBuf();
  } else
    lpPoint = new POINT[nPoints];
#else
  lpPoint = new POINT[nPoints];
#endif
  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    for (int i = 0; i < pLinearRing->GetNumPoints(); i++) {
      LPToDP(pLinearRing->GetX(i), pLinearRing->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
      // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
      // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y -
      // r,lpPoint[i].x + r ,lpPoint[i].y + r);
      DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
    }
  } else {
    for (int i = 0; i < nPoints; i++) {
      LPToDP(pLinearRing->GetX(i), pLinearRing->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
    }
  }

  MoveToEx(m_hCurDC, lpPoint[0].x, lpPoint[0].y, NULL);
  PolylineTo(m_hCurDC, lpPoint, nPoints);

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) < m_bufPool.GetPoolSize()) {
    m_bufPool.FreeAllBuf();
  } else
    SAFE_DELETE_A(lpPoint);
#else
  SAFE_DELETE_A(lpPoint);
#endif

  return ERR_NONE;
}

int RenderDeviceGDI::DrawPloygon(const Polygon *pPloygon) {
  int nAllPts = 0;
  const LinearRing *pLinerring = pPloygon->GetExteriorRing();

  int nExteriorPts = pLinerring->GetNumPoints();
  if (nExteriorPts < 2) return ERR_INVALID_PARAM;

  BOOL bRet = FALSE;

  nAllPts += nExteriorPts;

  int nInteriorRings = pPloygon->GetNumInteriorRings();
  int *nRings = new int[nInteriorRings + 1];
  nRings[0] = nExteriorPts;

  for (int i = 0; i < nInteriorRings; i++) {
    const LinearRing *pInteriorRing = pPloygon->GetInteriorRing(i);
    nRings[i + 1] = pInteriorRing->GetNumPoints();
    nAllPts += nRings[i + 1];
  }

  int i = 0;
  POINT *lpPoint = NULL;

#ifdef GDI_USE_BUFPOOL
  if (nAllPts * (sizeof(POINT)) < m_bufPool.GetPoolSize()) {
    m_bufPool.FreeAllBuf();
    lpPoint = (POINT *)m_bufPool.NewBuf();
  } else
    lpPoint = new POINT[nAllPts];
#else
  lpPoint = new POINT[nAllPts];
#endif

  int nCount = 0;
  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    for (int i = 0; i < nExteriorPts; i++, nCount++) {
      LPToDP(pLinerring->GetX(i), pLinerring->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
      // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
      // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y -
      // r,lpPoint[i].x + r ,lpPoint[i].y + r);
      DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
    }

    for (int i = 0; i < nInteriorRings; i++) {
      const LinearRing *pInteriorRing = pPloygon->GetInteriorRing(i);
      int nInteriorPts = pInteriorRing->GetNumPoints();
      for (int j = 0; j < nInteriorPts; ++j, nCount++) {
        LPToDP(pInteriorRing->GetX(i), pInteriorRing->GetY(i), lpPoint[i].x,
               lpPoint[i].y);
        // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
        // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y
        // - r,lpPoint[i].x + r ,lpPoint[i].y + r);
        DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
      }
    }

    bRet = ::PolyPolygon(m_hCurDC, lpPoint, nRings, nInteriorRings + 1);
  } else {
    for (int i = 0; i < nExteriorPts; i++, nCount++) {
      LPToDP(pLinerring->GetX(i), pLinerring->GetY(i), lpPoint[i].x,
             lpPoint[i].y);
    }

    for (int i = 0; i < nInteriorRings; i++) {
      const LinearRing *pInteriorRing = pPloygon->GetInteriorRing(i);
      int nInteriorPts = pInteriorRing->GetNumPoints();
      for (int j = 0; j < nInteriorPts; ++j, nCount++) {
        LPToDP(pInteriorRing->GetX(i), pInteriorRing->GetY(i), lpPoint[i].x,
               lpPoint[i].y);
      }
    }

    bRet = ::PolyPolygon(m_hCurDC, lpPoint, nRings, nInteriorRings + 1);
  }

#ifdef GDI_USE_BUFPOOL
  if (nAllPts * (sizeof(POINT)) < m_bufPool.GetPoolSize()) {
    m_bufPool.FreeAllBuf();
  } else
    SAFE_DELETE_A(lpPoint);
#else
  SAFE_DELETE_A(lpPoint);
#endif

  SAFE_DELETE_A(nRings);

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawTin(const Tin *pTin) {
  DrawTinLines(pTin);

  // if (m_rdPra.show_point)
  { DrawTinNodes(pTin); }

  return ERR_NONE;
}

//绘制Tin线
int RenderDeviceGDI::DrawTinLines(const Tin *pTin) {
  POINT lPt1, lPt2, lPt3;
  Point oPt1, oPt2, oPt3;

  Envelope envTri, envViewp;
  lRect lViewp;
  fRect fViewp;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  RectToEnvelope(envViewp, fViewp);

  for (int i = 0; i < pTin->GetTriangleCount(); i++) {
    Triangle tri = pTin->GetTriangle(i);

    if (!tri.bDelete) {
      oPt1 = pTin->GetPoint(tri.a);
      oPt2 = pTin->GetPoint(tri.b);
      oPt3 = pTin->GetPoint(tri.c);

      envTri.Merge(oPt1.GetX(), oPt1.GetY());
      envTri.Merge(oPt2.GetX(), oPt2.GetY());
      envTri.Merge(oPt3.GetX(), oPt3.GetY());

      if (envTri.Intersects(envViewp)) {
        LPToDP(oPt1.GetX(), oPt1.GetY(), lPt1.x, lPt1.y);
        LPToDP(oPt2.GetX(), oPt2.GetY(), lPt2.x, lPt2.y);
        LPToDP(oPt3.GetX(), oPt3.GetY(), lPt3.x, lPt3.y);

        MoveToEx(m_hCurDC, lPt1.x, lPt1.y, NULL);
        LineTo(m_hCurDC, lPt2.x, lPt2.y);
        LineTo(m_hCurDC, lPt3.x, lPt3.y);
        LineTo(m_hCurDC, lPt1.x, lPt1.y);
      }
    }
  }

  return ERR_NONE;
}

//绘制Tin节点
int RenderDeviceGDI::DrawTinNodes(const Tin *pTin) {
  POINT lPt;
  Point oPt;
  int r = m_rdPra.point_raduis;

  lRect lViewp;
  fRect fViewp;

  ViewportToRect(lViewp, m_Viewport);
  DRectToLRect(lViewp, fViewp);
  AjustfRect(fViewp);

  for (int i = 0; i < pTin->GetPointCount(); i++) {
    oPt = pTin->GetPoint(i);
    if (IsInfRect(oPt.GetX(), oPt.GetY(), fViewp)) {
      LPToDP(oPt.GetX(), oPt.GetY(), lPt.x, lPt.y);
      Ellipse(m_hCurDC, lPt.x - r, lPt.y - r, lPt.x + r, lPt.y + r);
    }
  }

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawGrid(const Grid *pGrid) {
  DrawGridLines(pGrid);

  // if (m_rdPra.show_point)
  { DrawGridNodes(pGrid); }

  return ERR_NONE;
}

//绘制网格线
int RenderDeviceGDI::DrawGridLines(const Grid *pGrid) {
  const Matrix2D<RawPoint> *pNodes = pGrid->GetGridNodes();

  int nM, nN;
  pGrid->GetSize(nM, nN);

  POINT lPt;

  for (int j = 0; j < nN; j++) {  //绘制列
    RawPoint rawPt = pNodes->GetElement(0, j);
    LPToDP(rawPt.x, rawPt.y, lPt.x, lPt.y);
    MoveToEx(m_hCurDC, lPt.x, lPt.y, NULL);
    for (int i = 0; i < nM; i++) {  //绘制行
      RawPoint rawPt1 = pNodes->GetElement(i, j);
      LPToDP(rawPt1.x, rawPt1.y, lPt.x, lPt.y);
      LineTo(m_hCurDC, lPt.x, lPt.y);
    }
  }

  for (int i = 0; i < nM; i++) {  //绘制列
    RawPoint rawPt = pNodes->GetElement(i, 0);
    LPToDP(rawPt.x, rawPt.y, lPt.x, lPt.y);
    MoveToEx(m_hCurDC, lPt.x, lPt.y, NULL);

    for (int j = 0; j < nN; j++) {  //绘制行
      RawPoint rawPt1 = pNodes->GetElement(i, j);
      LPToDP(rawPt1.x, rawPt1.y, lPt.x, lPt.y);
      LineTo(m_hCurDC, lPt.x, lPt.y);
    }
  }

  return ERR_NONE;
}

//绘制网格节点
int RenderDeviceGDI::DrawGridNodes(const Grid *pGrid) {
  const Matrix2D<RawPoint> *pNodes = pGrid->GetGridNodes();

  int nM, nN;
  pGrid->GetSize(nM, nN);

  int r = m_rdPra.point_raduis;
  POINT lPt;

  for (int j = 0; j < nN; j++) {
    for (int i = 0; i < nM; i++) {
      RawPoint rawPt = pNodes->GetElement(i, j);
      LPToDP(rawPt.x, rawPt.y, lPt.x, lPt.y);
      Ellipse(m_hCurDC, lPt.x - r, lPt.y - r, lPt.x + r, lPt.y + r);
    }
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawFan(const Fan *pFan) {
  long x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
  Point oStPoint, oEdbfPoint, oCtPoint;

  const Arc *pArc = pFan->GetArc();
  pArc->StartPoint(&oStPoint);
  pArc->EndPoint(&oEdbfPoint);
  pArc->GetCenterPoint(&oCtPoint);

  LPToDP(oStPoint.GetX(), oStPoint.GetY(), x3, y3);
  LPToDP(oEdbfPoint.GetX(), oEdbfPoint.GetY(), x4, y4);
  LPToDP(oCtPoint.GetX(), oCtPoint.GetY(), x5, y5);

  int dr = m_rdPra.point_raduis;
  float r = Distance(x4, y4, x5, y5);

  x1 = x5 - r;
  y1 = y5 - r;
  x2 = x5 + r;
  y2 = y5 + r;

  ::MoveToEx(m_hCurDC, x4, y4, NULL);
  LineTo(m_hCurDC, x5, y5);
  LineTo(m_hCurDC, x3, y3);
  ::Pie(m_hCurDC, x1, y1, x2, y2, x3, y3, x4, y4);

  Ellipse(m_hCurDC, x3 - dr, y3 - dr, x3 + dr, y3 + dr);
  Ellipse(m_hCurDC, x4 - dr, y4 - dr, x4 + dr, y4 + dr);
  Ellipse(m_hCurDC, x5 - dr, y5 - dr, x5 + dr, y5 + dr);

  if (m_rdPra.show_point) {
    // Rectangle(m_hCurDC,x3 - dr ,y3 - dr,x3 + dr ,y3 + dr);
    // Rectangle(m_hCurDC,x4 - dr ,y4 - dr,x4 + dr ,y4 + dr);
    // Rectangle(m_hCurDC,x5 - dr ,y5 - dr,x5 + dr ,y5 + dr);

    DrawCross(m_hCurDC, x3, y3, dr);
    DrawCross(m_hCurDC, x4, y4, dr);
    DrawCross(m_hCurDC, x5, y5, dr);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawArc(const Arc *pArc) {
  long x1, y1, x2, y2, x3, y3, x4, y4, x5, y5;
  Point oStPoint, oEdbfPoint, oCtPoint;

  pArc->StartPoint(&oStPoint);
  pArc->EndPoint(&oEdbfPoint);
  pArc->GetCenterPoint(&oCtPoint);

  LPToDP(oStPoint.GetX(), oStPoint.GetY(), x3, y3);
  LPToDP(oEdbfPoint.GetX(), oEdbfPoint.GetY(), x4, y4);
  LPToDP(oCtPoint.GetX(), oCtPoint.GetY(), x5, y5);

  int dr = m_rdPra.point_raduis;
  float r = Distance(x4, y4, x5, y5);

  x1 = x5 - r;
  y1 = y5 - r;
  x2 = x5 + r;
  y2 = y5 + r;

  //::MoveToEx(m_hCurDC,x4,y4,NULL);
  // LineTo(m_hCurDC,x5,y5);
  // LineTo(m_hCurDC,x3,y3);
  ::Arc(m_hCurDC, x1, y1, x2, y2, x3, y3, x4, y4);

  Ellipse(m_hCurDC, x3 - dr, y3 - dr, x3 + dr, y3 + dr);
  Ellipse(m_hCurDC, x4 - dr, y4 - dr, x4 + dr, y4 + dr);
  Ellipse(m_hCurDC, x5 - dr, y5 - dr, x5 + dr, y5 + dr);

  if (m_rdPra.show_point) {
    // Rectangle(m_hCurDC,x3 - dr ,y3 - dr,x3 + dr ,y3 + dr);
    // Rectangle(m_hCurDC,x4 - dr ,y4 - dr,x4 + dr ,y4 + dr);
    // Rectangle(m_hCurDC,x5 - dr ,y5 - dr,x5 + dr ,y5 + dr);

    DrawCross(m_hCurDC, x3, y3, dr);
    DrawCross(m_hCurDC, x4, y4, dr);
    DrawCross(m_hCurDC, x5, y5, dr);
  }

  return ERR_NONE;
}

//////////////////////////////////////////////////////////////////////////
int RenderDeviceGDI::DrawEllipse(float left, float top, float right,
                                 float bottom, bool bDP) {
  lRect lrect;

  fRect frect;
  frect.lb.x = left;
  frect.lb.y = bottom;
  frect.rt.x = right;
  frect.rt.y = top;

  if (!bDP)
    LRectToDRect(frect, lrect);
  else
    fRectTolRect(lrect, frect);

  Ellipse(m_hCurDC, lrect.lb.x, lrect.lb.y, lrect.rt.x, lrect.rt.y);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawRect(const fRect &rect, bool bDP) {
  lRect tmpRectDP;

  fRectTolRect(tmpRectDP, rect);

  if (!bDP) {
    fRect tmpRectLP;

    lRectTofRect(tmpRectLP, tmpRectDP);
    LRectToDRect(tmpRectLP, tmpRectDP);
  }

  MoveToEx(m_hCurDC, tmpRectDP.lb.x, tmpRectDP.lb.y, NULL);
  LineTo(m_hCurDC, tmpRectDP.rt.x, tmpRectDP.lb.y);
  LineTo(m_hCurDC, tmpRectDP.rt.x, tmpRectDP.rt.y);
  LineTo(m_hCurDC, tmpRectDP.lb.x, tmpRectDP.rt.y);
  LineTo(m_hCurDC, tmpRectDP.lb.x, tmpRectDP.lb.y);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLine(fPoint *pfPoints, int nCount, bool bDP) {
  int nPoints = nCount;
  if (nPoints < 2) return ERR_INVALID_PARAM;

  int i = 0;
  POINT *lpPoint = NULL;

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
    lpPoint = (POINT *)m_bufPool.NewBuf();
  } else
    lpPoint = new POINT[nPoints];
#else
  lpPoint = new POINT[nPoints];
#endif

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    if (!bDP) {
      for (int i = 0; i < nPoints; i++) {
        LPToDP(pfPoints[i].x, pfPoints[i].y, lpPoint[i].x, lpPoint[i].y);
        // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
        // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y
        // - r,lpPoint[i].x + r ,lpPoint[i].y + r);
        DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
      }
    } else {
      for (int i = 0; i < nPoints; i++) {
        lpPoint[i].x = pfPoints[i].x;
        lpPoint[i].y = pfPoints[i].y;
        // Ellipse(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y - r,lpPoint[i].x + r
        // ,lpPoint[i].y + r); Rectangle(m_hCurDC,lpPoint[i].x - r ,lpPoint[i].y
        // - r,lpPoint[i].x + r ,lpPoint[i].y + r);
        DrawCross(m_hCurDC, lpPoint[i].x, lpPoint[i].y, r);
      }
    }
  } else {
    if (!bDP) {
      for (int i = 0; i < nPoints; i++) {
        LPToDP(pfPoints[i].x, pfPoints[i].y, lpPoint[i].x, lpPoint[i].y);
      }
    } else {
      for (int i = 0; i < nPoints; i++) {
        lpPoint[i].x = pfPoints[i].x;
        lpPoint[i].y = pfPoints[i].y;
      }
    }
  }

  MoveToEx(m_hCurDC, lpPoint[0].x, lpPoint[0].y, NULL);
  PolylineTo(m_hCurDC, lpPoint, nPoints);

#ifdef GDI_USE_BUFPOOL
  if (nPoints * (sizeof(POINT)) <
      (m_bufPool.GetBufCount() * m_bufPool.GetSizePerBuf())) {
    m_bufPool.FreeAllBuf();
  } else
    SAFE_DELETE_A(lpPoint);
#else
  SAFE_DELETE_A(lpPoint);
#endif

  return ERR_NONE;
}

int RenderDeviceGDI::DrawLine(const fPoint &ptA, const fPoint &ptB, bool bDP) {
  lPoint pt1(ptA.x, ptA.y), pt2(ptB.x, ptB.y);

  if (!bDP) {
    LPToDP(ptA.x, ptA.y, pt1.x, pt1.y);
    LPToDP(ptB.x, ptB.y, pt2.x, pt2.y);
  }

  MoveToEx(m_hCurDC, pt1.x, pt1.y, NULL);
  LineTo(m_hCurDC, pt2.x, pt2.y);

  return ERR_NONE;
}

int RenderDeviceGDI::DrawText(const char *szAnno, float fangel, float fCHeight,
                              float fCWidth, float fCSpace, const fPoint &point,
                              bool bDP) {
  if (szAnno == NULL) return ERR_INVALID_PARAM;

  fCHeight *= m_fblc;
  fCWidth *= m_fblc;
  fCSpace *= m_fblc;

  unsigned char c1, c2;
  fPoint pt;
  long x, y;
  char bz[4];
  const char *ls1;
  ls1 = szAnno;

  lRect tmpRectDP;

  if (!bDP) {
    LPToDP(point.x, point.y, x, y);
    pt.x = x;
    pt.y = y;
  } else {
    pt.x = point.x;
    pt.y = point.y;
  }

  pt.x -= 2 * fCHeight * sin(fangel);
  pt.y -= 2 * fCHeight * cos(fangel);

  int nStrLength = (int)strlen(ls1);
  while (nStrLength > 0) {
    c1 = *ls1;
    c2 = *(ls1 + 1);
    if (c1 > 127 && c2 > 127)  //如果下一个字符是汉字
    {
      strncpy(bz, ls1, 2);
      bz[2] = 0;
      ls1 = ls1 + 2;
      TextOut(m_hCurDC, pt.x, pt.y, (LPCSTR)bz, 2);
      nStrLength -= 2;
      pt.x += (fCWidth * 2 + fCSpace) * cos(fangel);
      pt.y += (fCWidth * 2 + fCSpace) * sin(fangel);
    } else {
      strncpy(bz, ls1, 1);
      bz[1] = 0;
      ls1++;
      TextOut(m_hCurDC, pt.x, pt.y, (LPCSTR)bz, 1);
      nStrLength -= 1;

      pt.x += (fCWidth + fCSpace / 2.) * cos(fangel);
      pt.y += (fCWidth + fCSpace / 2.) * sin(fangel);
    }
  }

  if (m_rdPra.show_point) {
    int r = m_rdPra.point_raduis;
    long lX, lY;
    if (!bDP) {
      LPToDP(point.x, point.y, lX, lY);
    } else {
      lX = point.x;
      lY = point.y;
    }

    // Ellipse(m_hCurDC,lX - r ,lY - r,lX + r ,lY + r);
    // Rectangle(m_hCurDC,lX - r,lY - r,lX + r,lY + r);
    DrawCross(m_hCurDC, lX, lY, r);
  }

  return ERR_NONE;
}

int RenderDeviceGDI::DrawImage(const char *szImageBuf, int nImageBufSize,
                               const fRect &frect, long lCodeType,
                               eRDBufferLayer eMRDBufLyr) {
  long lRtn = ERR_FAILURE;

  lRect lrt;
  LRectToDRect(frect, lrt);

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      lRtn = m_smtMapRenderBuf.DrawImage(szImageBuf, nImageBufSize, lCodeType,
                                         lrt.lb.x, lrt.rt.y, lrt.Width(),
                                         lrt.Height());
    } break;
  }

  return lRtn;
}

int RenderDeviceGDI::StrethImage(const char *szImageBuf, int nImageBufSize,
                                 const fRect &frect, long lCodeType,
                                 eRDBufferLayer eMRDBufLyr) {
  long lRtn = ERR_FAILURE;

  lRect lrt;

  LRectToDRect(frect, lrt);

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      lRtn = m_smtMapRenderBuf.StrethImage(szImageBuf, nImageBufSize, lCodeType,
                                           lrt.lb.x, lrt.rt.y, lrt.Width(),
                                           lrt.Height());
    } break;
  }

  return lRtn;
}

int RenderDeviceGDI::SaveImage(const char *szFilePath,
                               eRDBufferLayer eMRDBufLyr, bool bBgTransparent) {
  long lRtn = ERR_FAILURE;

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      lRtn = m_smtMapRenderBuf.Save2Image(szFilePath, bBgTransparent);
    } break;
  }

  return lRtn;
}

int RenderDeviceGDI::Save2ImageBuf(char *&szImageBuf, long &lImageBufSize,
                                   long lCodeType, eRDBufferLayer eMRDBufLyr,
                                   bool bBgTransparent) {
  long lRtn = ERR_FAILURE;

  switch (eMRDBufLyr) {
    case MRD_BL_MAP: {
      lRtn = m_smtMapRenderBuf.Save2ImageBuf(szImageBuf, lImageBufSize,
                                             lCodeType, bBgTransparent);
    } break;
  }

  return lRtn;
}

int RenderDeviceGDI::FreeImageBuf(char *&szImageBuf) {
  return RenderBuf::FreeImageBuf(szImageBuf);
}
}  // namespace gfx2d