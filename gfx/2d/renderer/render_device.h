#ifndef GFX_2D_RENDERER_RENDER_DEVICE_H
#define GFX_2D_RENDERER_RENDER_DEVICE_H

namespace gfx2d {
enum eRDBufferLayer { MRD_BL_MAP, MRD_BL_DYNAMIC, MRD_BL_QUICK, MRD_BL_DIRECT };

class RenderDevice {
 public:
  RenderDevice(HINSTANCE hInst)
      : m_rBaseApi(RD_GDI),
        m_hInst(hInst),
        m_strLogName(""),
        m_hWnd(NULL),
        m_nMapMode(MM_TEXT),
        m_fblc(1.) {
    ;
  }

  virtual ~RenderDevice(void) {}

 public:
  virtual int Init(HWND hWnd, const char *logname) = 0;
  virtual int Destroy(void) = 0;
  virtual int Release(void) = 0;

  virtual int Resize(int orgx, int orgy, int cx, int cy) = 0;

  void SetViewport(const Viewport &viewport) { m_Viewport = viewport; }
  Viewport GetViewport(void) const { return m_Viewport; }

  void SetWindowport(const Windowport &windowport) {
    m_Windowport = windowport;
  }
  Windowport GetWindowport(void) const { return m_Windowport; }

  void SetCurDrawingOrg(const lPoint &ptPos) { m_curDrawingOrg = ptPos; }
  lPoint GetCurDrawingOrg(void) const { return m_curDrawingOrg; }

  void SetMapMode(int nMode) { m_nMapMode = nMode; }
  int GetMapMode(void) const { return m_nMapMode; }

  void SetRenderPra(const RenderOptions &options) { options_ = options; }
  RenderOptions GetRenderPra(void) const { return options_; }

  inline double GetBlc(void) const { return m_fblc; }

 public:
  virtual int Lock() = 0;
  virtual int Unlock() = 0;

  virtual int Refresh(void) = 0;
  virtual int Refresh(const Map *pMap, fRect rect) = 0;
  virtual int RefreshDirectly(const Map *pMap, lRect rect,
                              bool bRealTime = false) = 0;

  virtual int ZoomMove(const Map *pMap, fPoint dbfPointOffset,
                       bool bRealTime = false) = 0;
  virtual int ZoomScale(const Map *pMap, lPoint orgPoint, float fscale,
                        bool bRealTime = false) = 0;
  virtual int ZoomToRect(const Map *pMap, fRect rect,
                         bool bRealTime = false) = 0;

  virtual int Timer() = 0;

 public:
  virtual int LPToDP(float x, float y, long &X, long &Y) const = 0;
  virtual int DPToLP(LONG X, LONG Y, float &x, float &y) const = 0;
  virtual int LRectToDRect(const fRect &frect, lRect &lrect) const = 0;
  virtual int DRectToLRect(const lRect &lrect, fRect &frect) const = 0;

 public:
  virtual int BeginRender(eRDBufferLayer eMRDBufLyr, bool bClear = false,
                          const Style *pStyle = NULL, int op = R2_COPYPEN) = 0;
  virtual int EndRender(eRDBufferLayer eMRDBufLyr) = 0;

 public:
  virtual int RenderMap(void) = 0;

  virtual int RenderMap(const Map *pMap, int op = R2_COPYPEN) = 0;
  virtual int RenderLayer(const Layer *pLayer, int op = R2_COPYPEN) = 0;
  virtual int RenderLayer(const VectorLayer *pLayer, int op = R2_COPYPEN) = 0;
  virtual int RenderLayer(const RasterLayer *pLayer, int op = R2_COPYPEN) = 0;
  virtual int RenderLayer(const TileLayer *pLayer, int op = R2_COPYPEN) = 0;
  virtual int RenderFeature(const Feature *pFeature, int op = R2_COPYPEN) = 0;
  virtual int RenderGeometry(const Geometry *pGeom, const Style *pStyle,
                             int op = R2_COPYPEN) = 0;

  virtual int DrawMultiLineString(const MultiLineString *pMultiLinestring) = 0;
  virtual int DrawMultiPoint(const Style *pStyle,
                             const MultiPoint *pMultiPoint) = 0;
  virtual int DrawMultiPolygon(const MultiPolygon *pMultiPolygon) = 0;

  virtual int DrawPoint(const Style *pStyle, const Point *pPoint) = 0;
  virtual int DrawAnno(const char *szAnno, float fangel, float fCHeight,
                       float fCWidth, float fCSpace, const Point *pPoint) = 0;
  virtual int DrawSymbol(HICON hIcon, long lHeight, long lWhidth,
                         const Point *pPoint) = 0;

  virtual int DrawLineString(const LineString *pLinestring) = 0;
  virtual int DrawLineSpline(const Spline *pSpline) = 0;
  virtual int DrawLinearRing(const LinearRing *pLinearRing) = 0;
  virtual int DrawPloygon(const Polygon *pPloygon) = 0;

  virtual int DrawTin(const Tin *pTin) = 0;
  virtual int DrawGrid(const Grid *pGrid) = 0;
  virtual int DrawArc(const Arc *pArc) = 0;
  virtual int DrawFan(const Fan *pFan) = 0;

 public:
  virtual int DrawEllipse(float left, float top, float right, float bottom,
                          bool bDP = false) = 0;
  virtual int DrawRect(const fRect &rect, bool bDP = false) = 0;
  virtual int DrawLine(fPoint *pfPoints, int nCount, bool bDP = false) = 0;
  virtual int DrawLine(const fPoint &ptA, const fPoint &ptB,
                       bool bDP = false) = 0;
  virtual int DrawText(const char *szAnno, float fangel, float fCHeight,
                       float fCWidth, float fCSpace, const fPoint &point,
                       bool bDP = false) = 0;
  virtual int DrawImage(const char *szImageBuf, int nImageBufSize,
                        const fRect &frect, long lCodeType,
                        eRDBufferLayer eMRDBufLyr = MRD_BL_MAP) = 0;
  virtual int StrethImage(const char *szImageBuf, int nImageBufSize,
                          const fRect &frect, long lCodeType,
                          eRDBufferLayer eMRDBufLyr = MRD_BL_MAP) = 0;

 public:
  virtual int SaveImage(const char *szFilePath,
                        eRDBufferLayer eMRDBufLyr = MRD_BL_MAP,
                        bool bBgTransparent = false) = 0;
  virtual int Save2ImageBuf(char *&szImageBuf, long &lImageBufSize,
                            long lCodeType,
                            eRDBufferLayer eMRDBufLyr = MRD_BL_MAP,
                            bool bBgTransparent = false) = 0;
  virtual int FreeImageBuf(char *&szImageBuf) = 0;

 protected:
  HINSTANCE m_hInst;
  RenderBaseApi m_rBaseApi;
  string m_strLogName;

  Viewport m_Viewport;
  Windowport m_Windowport;
  float m_fblc;

  HWND m_hWnd;
  int m_nMapMode;
  RenderOptions options_;

  lPoint m_curDrawingOrg;
};

typedef RenderDevice *LPRENDERDEVICE;

#ifdef __cplusplus
extern "C" {
#endif
int SMT_EXPORT_DLL CreateRenderDevice(HINSTANCE hInst,
                                      LPRENDERDEVICE &pMrdDevice);
int SMT_EXPORT_DLL DestroyRenderDevice(LPRENDERDEVICE &pMrdDevice);

typedef HRESULT (*_CreateRenderDevice)(HINSTANCE hInst,
                                       LPRENDERDEVICE &pMrdDevice);
typedef HRESULT (*_DestroyRenderDevice)(LPRENDERDEVICE &pMrdDevice);

#ifdef __cplusplus
}
#endif
}  // namespace gfx2d

#endif  //GFX_2D_RENDERER_RENDER_DEVICE_H