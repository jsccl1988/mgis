#ifndef GFX_2D_RENDER_DEVICE_BUFFER_H
#define GFX_2D_RENDER_DEVICE_BUFFER_H

#include <wingdi.h>
#include <wtypes.h>

namespace gfx2d {
class RenderBuffer {
 public:
  enum SwapType {
    BLT_STRETCH,
    BLT_TRANSPARENT,
  };
  RenderBuffer(void);
  RenderBuffer(HWND hWnd);
  ~RenderBuffer(void);

 public:
  inline HWND GetHWND() const { return m_hWnd; }
  inline long SetHWND(HWND hWnd);

  long SetSize(int cx, int cy);
  inline long GetWidth(void) const { return m_nWidth; }
  inline long GetHeight(void) const { return m_nHeight; }

  inline HBITMAP GetBuffer(void) { return m_hPaintBuf; }
  long ShareBuffer(RenderBuffer &rbSrc);

  long Swap(int destOrgx, int destOrgy, int destW, int destH, int srcOrgx,
               int srcOrgy, int op = SRCCOPY);
  long Swap(int destOrgx, int destOrgy, int destW, int destH, int srcOrgx,
               int srcOrgy, int srcW, int srcH,
               eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
               COLORREF clr = RGB(255, 255, 255));

  long Swap(RenderBuffer &rbTarget, int destOrgx, int destOrgy, int destW,
               int destH, int srcOrgx, int srcOrgy, int op = SRCCOPY);
  long Swap(RenderBuffer &rbTarget, int destOrgx, int destOrgy, int destW,
               int destH, int srcOrgx, int srcOrgy, int srcW, int srcH,
               eSwapType type = BLT_TRANSPARENT, int op = SRCCOPY,
               COLORREF clr = RGB(255, 255, 255));

  HDC PrepareDC(bool bClip = true);

  long EndDC(void);

  long ClearBuf(int x, int y, int w, int h, COLORREF clr = RGB(255, 255, 255));

  //////////////////////////////////////////////////////////////////////////
  long DrawImage(const char *szImageBuffer, int nImageBufferSize, long lCodeType,
                 long x = 0, long y = 0, long cx = -1, long cy = -1);
  long StrethImage(const char *szImageBuffer, int nImageBufferSize, long lCodeType,
                   long xoffset, long yoffset, long xsize, long ysize,
                   DWORD dwRop = SRCCOPY);

  long Save2Image(const char *szFilePath, bool bBgTransparent = false);
  long Save2ImageBuffer(char *&szImageBuffer, long &lImageBufferSize, long lCodeType,
                     bool bBgTransparent = false);

  static long Save2Image(HBITMAP hBitMap, const char *szFilePath,
                         bool bBgTransparent = false);
  static long Save2ImageBuffer(HBITMAP hBitMap, char *&szImageBuffer,
                            long &lImageBufferSize, long lCodeType,
                            bool bBgTransparent = false);
  static long FreeImageBuffer(char *&szImageBuffer);

 public:
  RenderBuffer &operator=(const RenderBuffer &other);

 protected:
  HWND m_hWnd;
  HDC m_hPaintDC;
  HBITMAP m_hOldPaintBuffer;
  HBITMAP m_hPaintBuf;

  long m_nWidth;
  long m_nHeight;
  bool m_bOnwerBuf;
};
}  // namespace gfx2d
#endif  // GFX_2D_RENDER_DEVICE_BUFFER_H