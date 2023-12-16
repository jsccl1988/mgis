#ifndef GFX_2D_RENDER_DEVICE_BUFFER_H
#define GFX_2D_RENDER_DEVICE_BUFFER_H

#include <wingdi.h>
#include <wtypes.h>

namespace gfx2d {
class RenderBuffer {
 public:
  enum SwapType {
    BLT_STRETCH,      //拉伸
    BLT_TRANSPARENT,  //透明
  };
  RenderBuffer(void);
  RenderBuffer(HWND hWnd);
  ~RenderBuffer(void);

 public:
  inline HWND GetWnd() const { return m_hWnd; }
  inline long SetWnd(HWND hWnd);

  long SetBufSize(int cx, int cy);
  inline long GetBufWidth(void) const { return m_nBufWidth; }
  inline long GetBufHeight(void) const { return m_nBufHeight; }

  inline HBITMAP GetBuf(void) { return m_hPaintBuf; }
  long ShareBuf(RenderBuffer &rbSrc);

  long SwapBuf(int destOrgx, int destOrgy, int destW, int destH, int srcOrgx,
               int srcOrgy, int op = SRCCOPY);
  long SwapBuf(int destOrgx, int destOrgy, int destW, int destH, int srcOrgx,
               int srcOrgy, int srcW, int srcH,
               eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
               COLORREF clr = RGB(255, 255, 255));

  long SwapBuf(RenderBuffer &rbTarget, int destOrgx, int destOrgy, int destW,
               int destH, int srcOrgx, int srcOrgy, int op = SRCCOPY);
  long SwapBuf(RenderBuffer &rbTarget, int destOrgx, int destOrgy, int destW,
               int destH, int srcOrgx, int srcOrgy, int srcW, int srcH,
               eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
               COLORREF clr = RGB(255, 255, 255));

  HDC PrepareDC(bool bClip = true);

  long EndDC(void);

  long ClearBuf(int x, int y, int w, int h, COLORREF clr = RGB(255, 255, 255));

  //////////////////////////////////////////////////////////////////////////
  long DrawImage(const char *szImageBuf, int nImageBufSize, long lCodeType,
                 long x = 0, long y = 0, long cx = -1, long cy = -1);
  long StrethImage(const char *szImageBuf, int nImageBufSize, long lCodeType,
                   long xoffset, long yoffset, long xsize, long ysize,
                   DWORD dwRop = SRCCOPY);

  long Save2Image(const char *szFilePath, bool bBgTransparent = false);
  long Save2ImageBuf(char *&szImageBuf, long &lImageBufSize, long lCodeType,
                     bool bBgTransparent = false);

  static long Save2Image(HBITMAP hBitMap, const char *szFilePath,
                         bool bBgTransparent = false);
  static long Save2ImageBuf(HBITMAP hBitMap, char *&szImageBuf,
                            long &lImageBufSize, long lCodeType,
                            bool bBgTransparent = false);
  static long FreeImageBuf(char *&szImageBuf);

 public:
  RenderBuffer &operator=(const RenderBuffer &other);

 protected:
  HWND m_hWnd;             //窗口句柄
  HDC m_hPaintDC;          //临时 DC
  HBITMAP m_hOldPaintBuf;  //临时 BMP
  HBITMAP m_hPaintBuf;     // buf BMP

  long m_nBufWidth;   // buf宽度
  long m_nBufHeight;  // buf高度
  bool m_bOnwerBuf;   //拥有权
};
}  // namespace gfx2d
#endif  // GFX_2D_RENDER_DEVICE_BUFFER_H