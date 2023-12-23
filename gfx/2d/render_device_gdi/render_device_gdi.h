#ifndef GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H
#define GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H

#include <wingdi.h>
#include <wtypes.h>

#include "gfx/2d/renderer/render_device.h"

namespace gfx2d {
class RenderDeviceGDI : public RenderDevice {
 public:
  RenderDeviceGDI(HINSTANCE hInst);
  virtual ~RenderDeviceGDI(void);

  int Init(HWND hWnd, const char *logname);
  int Destroy(void);
  int Release(void);

  int Resize(int orgx, int orgy, int cx, int cy);

 public:
  int Lock();
  int Unlock();

  int Refresh(void);
  int Refresh(const Map *pMap, fRect rect);
  int RefreshDirectly(const Map *pMap, lRect rect, bool bRealTime = false);

  int ZoomMove(const Map *pMap, fPoint dbfPointOffset, bool bRealTime = false);
  int ZoomScale(const Map *pMap, lPoint orgPoint, float fscale,
                bool bRealTime = false);
  int ZoomToRect(const Map *pMap, fRect rect, bool bRealTime = false);

  int Timer();

 public:
  int LPToDP(float x, float y, long &X, long &Y) const;
  int DPToLP(LONG X, LONG Y, float &x, float &y) const;
  int LRectToDRect(const fRect &frect, lRect &lrect) const;
  int DRectToLRect(const lRect &lrect, fRect &frect) const;

  int RenderMap(const Map *pMap, int x, int y, int w, int h,
                int op = R2_COPYPEN);

  int RenderMap(void);

 public:
  int BeginRender(eRDBufferLayer eMRDBufLyr, bool bClear = false,
                  const Style *pStyle = NULL, int op = R2_COPYPEN);
  int EndRender(eRDBufferLayer eMRDBufLyr);

  int RenderMap(const Map *pMap, int op = R2_COPYPEN);
  int RenderLayer(const Layer *pLayer,
                  int op = R2_COPYPEN);  // 1-16 R2_BLACK-R2_WHITE
  int RenderLayer(const VectorLayer *pLayer,
                  int op = R2_COPYPEN);  // 1-16 R2_BLACK-R2_WHITE
  int RenderLayer(const RasterLayer *pLayer,
                  int op = R2_COPYPEN);  // 1-16 R2_BLACK-R2_WHITE
  int RenderLayer(const TileLayer *pLayer,
                  int op = R2_COPYPEN);  // 1-16 R2_BLACK-R2_WHITE
  int RenderFeature(const Feature *pFeature, int op = R2_COPYPEN);
  int RenderGeometry(const Geometry *pGeom, const Style *pStyle,
                     int op = R2_COPYPEN);

 public:
  int DrawMultiLineString(const MultiLineString *pMultiLinestring);
  int DrawMultiPoint(const Style *pStyle, const MultiPoint *pMultiPoint);
  int DrawMultiPolygon(const MultiPolygon *pMultiPolygon);

  int DrawPoint(const Style *pStyle, const Point *pPoint);
  int DrawAnno(const char *szAnno, float fangel, float fCHeight, float fCWidth,
               float fCSpace, const Point *pPoint);
  int DrawSymbol(HICON hIcon, long lHeight, long lWhidth, const Point *pPoint);

  int DrawLineString(const LineString *pLinestring);
  int DrawLineSpline(const Spline *pSpline);
  int DrawLinearRing(const LinearRing *pLinearRing);
  int DrawPloygon(const Polygon *pPloygon);

  int DrawTin(const Tin *pTin);
  int DrawGrid(const Grid *pGrid);
  int DrawArc(const Arc *pArc);
  int DrawFan(const Fan *pFan);

 public:
  int DrawEllipse(float left, float top, float right, float bottom,
                  bool bDP = false);
  int DrawRect(const fRect &rect, bool bDP = false);
  int DrawLine(fPoint *pfPoints, int nCount, bool bDP = false);
  int DrawLine(const fPoint &ptA, const fPoint &ptB, bool bDP = false);
  int DrawText(const char *szAnno, float fangel, float fCHeight, float fCWidth,
               float fCSpace, const fPoint &point, bool bDP = false);
  int DrawImage(const char *szImageBuffer, int nImageBufferSize, const fRect &frect,
                long lCodeType, eRDBufferLayer eMRDBufLyr = MRD_BL_MAP);
  int StrethImage(const char *szImageBuffer, int nImageBufferSize, const fRect &frect,
                  long lCodeType, eRDBufferLayer eMRDBufLyr = MRD_BL_MAP);

 public:
  int SaveImage(const char *szFilePath, eRDBufferLayer eMRDBufLyr = MRD_BL_MAP,
                bool bBgTransparent = false);
  int Save2ImageBuffer(char *&szImageBuffer, long &lImageBufferSize, long lCodeType,
                    eRDBufferLayer eMRDBufLyr = MRD_BL_MAP,
                    bool bBgTransparent = false);
  int FreeImageBuffer(char *&szImageBuffer);

 private:
  int DrawTinLines(const Tin *pTin);
  int DrawTinNodes(const Tin *pTin);

  int DrawGridLines(const Grid *pGrid);
  int DrawGridNodes(const Grid *pGrid);

 protected:
  int PrepareForDrawing(const Style *pStyle, int nDrawMode = R2_COPYPEN);
  int EndDrawing();

 protected:
  HFONT m_hFont;
  HPEN m_hPen;
  HBRUSH m_hBrush;
  HICON m_hIcon;

  HFONT m_hOldFont;
  HPEN m_hOldPen;
  HBRUSH m_hOldBrush;

  HDC m_hCurDC;
  bool m_bCurUseStyle;
  bool m_bLockStyle;

  char m_szAnno[2000];
  float m_fAnnoAngle;
  int m_nFeatureType;

  Viewport m_virViewport1;  //虚拟视口，屏幕图像映射使用
  Viewport m_virViewport2;  //虚拟视口，屏幕图像映射使用

  LONGLONG m_llLastRedrawCmdStamp;  //上次重绘命令的cpu时间戳
  LONGLONG m_llLastRedrawStamp;     //上次重绘的cpu时间戳
  bool m_bRedraw;                   //延时绘制

  RenderBuf m_smtMapRenderBuf;
  RenderBuf m_smtRenderBuf;

  BufPool m_bufPool;

  const Map *m_pCurMap;
  int m_nOX, m_nOY, m_nWidth, m_nHeight, m_nOp;

 protected:
  CSLock m_cslock;
};
}  // namespace gfx2d

#endif  //GFX_2D_RENDER_DEVICE_RENDER_DEVICE_GDI_H