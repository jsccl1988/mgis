#include "gfx/2d/render_device_gdi/render_buffer.h"

#include "ximage.h"

#pragma comment(lib, "Msimg32.lib")  // TransparentBlt

namespace gfx2d {
RenderBuffer::RenderBuffer(HWND hWnd) : m_hWnd(hWnd), m_bOnwerBuf(true) {}
RenderBuffer::RenderBuffer(void) : m_hWnd(NULL), m_bOnwerBuf(true) {}

RenderBuffer::~RenderBuffer(void) {
  if (m_bOnwerBuf && m_hPaintBuf) {
    DeleteObject(m_hPaintBuf);
    m_hPaintBuf = NULL;
  }
}

//////////////////////////////////////////////////////////////////////////
inline long RenderBuffer::SetWnd(HWND hWnd) {
  if (!m_bOnwerBuf) return SMT_ERR_FAILURE;
  m_hWnd = hWnd;
  return SMT_ERR_NONE;
}

long RenderBuffer::SetBufSize(int cx, int cy) {
  if (!m_bOnwerBuf || cx < 1 || cy < 1) return SMT_ERR_FAILURE;

  if (m_hPaintBuf) {
    DeleteObject(m_hPaintBuf);
    m_hPaintBuf = NULL;
  }

  HDC hDC = GetDC(m_hWnd);
  m_nBufWidth = cx;
  m_nBufHeight = cy;
  m_hPaintBuf = CreateCompatibleBitmap(hDC, m_nBufWidth, m_nBufHeight);

  if (NULL == m_hPaintBuf) return SMT_ERR_FAILURE;

  ClearBuf(0, 0, m_nBufWidth, m_nBufHeight);

  ::ReleaseDC(m_hWnd, hDC);

  return SMT_ERR_NONE;
}

long RenderBuffer::ShareBuf(RenderBuffer &rbSrc) {
  if (m_bOnwerBuf && m_hPaintBuf) {
    DeleteObject(m_hPaintBuf);
    m_hPaintBuf = NULL;
  }

  this->SetWnd(rbSrc.GetWnd());
  this->m_nBufHeight = rbSrc.GetBufHeight();
  this->m_nBufWidth = rbSrc.GetBufWidth();
  this->m_hPaintBuf = rbSrc.GetBuf();

  m_bOnwerBuf = false;

  return SMT_ERR_NONE;
}

long RenderBuffer::ClearBuf(int x, int y, int w, int h, COLORREF clr) {
  if (w < 1 || h < 1) return SMT_ERR_FAILURE;

  RECT rect;
  rect.left = x;
  rect.bottom = y;
  rect.right = x + w;
  rect.top = y + h;

  HDC hDC = GetDC(m_hWnd);
  HDC hPaintBufDC = CreateCompatibleDC(hDC);
  HBITMAP hPrePaintBuf = (HBITMAP)::SelectObject(hPaintBufDC, m_hPaintBuf);

  HBRUSH hBrush = CreateSolidBrush(clr);
  HBRUSH hOldBrush = (HBRUSH)::SelectObject(hPaintBufDC, hBrush);

  ::FillRect(hPaintBufDC, &rect, hBrush);

  ::SelectObject(hPaintBufDC, hOldBrush);
  ::DeleteObject(hBrush);

  ::SelectObject(hPaintBufDC, hPrePaintBuf);
  ::DeleteObject(hPaintBufDC);

  ::ReleaseDC(m_hWnd, hDC);

  return SMT_ERR_NONE;
}

long RenderBuffer::SwapBuf(int destOrgx, int destOrgy, int destW, int destH,
                           int srcOrgx, int srcOrgy, int op) {
  if (m_hPaintBuf == NULL) return SMT_ERR_FAILURE;

  HDC hDC = GetDC(m_hWnd);
  HDC hSrcDC = this->PrepareDC(false);

  BOOL bRet = ::BitBlt(hDC, destOrgx, destOrgy, destW, destH, hSrcDC, srcOrgx,
                       srcOrgy, op);

  this->EndDC();
  ::ReleaseDC(m_hWnd, hDC);

  if (bRet)
    return SMT_ERR_NONE;
  else
    return SMT_ERR_FAILURE;
}

long RenderBuffer::SwapBuf(int destOrgx, int destOrgy, int destW, int destH,
                           int srcOrgx, int srcOrgy, int srcW, int srcH,
                           eSwapType type, int op, COLORREF clr) {
  if (m_hPaintBuf == NULL) return SMT_ERR_FAILURE;

  HDC hDC = GetDC(m_hWnd);
  HDC hSrcDC = this->PrepareDC(false);
  BOOL bRet = TRUE;

  switch (type) {
    case BLT_STRETCH:
      bRet = ::StretchBlt(hDC, destOrgx, destOrgy, destW, destH, hSrcDC,
                          srcOrgx, srcOrgy, srcW, srcH, op);
      break;
    case BLT_TRANSPARENT:
      bRet = ::TransparentBlt(hDC, destOrgx, destOrgy, destW, destH, hSrcDC,
                              srcOrgx, srcOrgy, destW, destH, clr);
      break;
  }

  this->EndDC();
  ::ReleaseDC(m_hWnd, hDC);

  if (bRet)
    return SMT_ERR_NONE;
  else
    return SMT_ERR_FAILURE;
}

long RenderBuffer::SwapBuf(RenderBuffer &rbTarget, int destOrgx, int destOrgy,
                           int destW, int destH, int srcOrgx, int srcOrgy,
                           int op) {
  if (m_hPaintBuf == NULL) return SMT_ERR_FAILURE;

  HDC hTargetDC = rbTarget.PrepareDC(false);
  HDC hSrcDC = this->PrepareDC(false);

  BOOL bRet = ::BitBlt(hTargetDC, destOrgx, destOrgy, destW, destH, hSrcDC,
                       srcOrgx, srcOrgy, op);

  this->EndDC();
  rbTarget.EndDC();

  if (bRet)
    return SMT_ERR_NONE;
  else
    return SMT_ERR_FAILURE;
}

long RenderBuffer::SwapBuf(RenderBuffer &rbTarget, int destOrgx, int destOrgy,
                           int destW, int destH, int srcOrgx, int srcOrgy,
                           int srcW, int srcH, eSwapType type, int op,
                           COLORREF clr) {
  if (m_hPaintBuf == NULL) return SMT_ERR_FAILURE;

  HDC hTargetDC = rbTarget.PrepareDC(false);
  HDC hSrcDC = this->PrepareDC(false);

  BOOL bRet = TRUE;

  switch (type) {
    case BLT_STRETCH:
      bRet = ::StretchBlt(hTargetDC, destOrgx, destOrgy, destW, destH, hSrcDC,
                          srcOrgx, srcOrgy, srcW, srcH, op);
      break;
    case BLT_TRANSPARENT:
      bRet = ::TransparentBlt(hTargetDC, destOrgx, destOrgy, destW, destH,
                              hSrcDC, srcOrgx, srcOrgy, srcW, srcH, clr);
      break;
  }

  this->EndDC();
  rbTarget.EndDC();

  if (bRet)
    return SMT_ERR_NONE;
  else
    return SMT_ERR_FAILURE;
}

HDC RenderBuffer::PrepareDC(bool bClip) {
  HDC hDC = GetDC(m_hWnd);
  m_hPaintDC = CreateCompatibleDC(hDC);
  ::ReleaseDC(m_hWnd, hDC);

  m_hOldPaintBuf = (HBITMAP)SelectObject(m_hPaintDC, m_hPaintBuf);

  if (bClip) {
    RECT rtClip;
    HRGN hNewRgn = ::CreateRectRgn(0, 0, m_nBufWidth, m_nBufHeight);
    ::GetClipBox(m_hPaintDC, &rtClip);
    ::SelectClipRgn(m_hPaintDC, hNewRgn);
    ::DeleteObject(hNewRgn);
  }

  return m_hPaintDC;
}

long RenderBuffer::EndDC(void) {
  if (m_hPaintDC) {
    SelectObject(m_hPaintDC, m_hOldPaintBuf);
    DeleteDC(m_hPaintDC);

    m_hPaintDC = NULL;
  }

  return SMT_ERR_NONE;
}

RenderBuffer &RenderBuffer::operator=(const RenderBuffer &other) {
  if (&other == this) return *this;

  if (m_bOnwerBuf && NULL != m_hPaintBuf) {
    DeleteObject(m_hPaintBuf);
    m_hPaintBuf = NULL;
  }

  this->SetWnd(other.GetWnd());
  this->SetBufSize(other.GetBufWidth(), other.GetBufHeight());
  return *this;
}

//////////////////////////////////////////////////////////////////////////
long RenderBuffer::DrawImage(const char *szImageBuf, int nImageBufSize,
                             long lCodeType, long x, long y, long cx, long cy) {
  if (NULL == szImageBuf || 0 == nImageBufSize) return SMT_ERR_INVALID_PARAM;

  CxImage tmpImage;
  tmpImage.Decode((BYTE *)szImageBuf, nImageBufSize, lCodeType);

  HDC hDC = PrepareDC();
  tmpImage.Draw(hDC, x, y, cx, cy);
  EndDC();

  return SMT_ERR_NONE;
}

long RenderBuffer::StrethImage(const char *szImageBuf, int nImageBufSize,
                               long lCodeType, long xoffset, long yoffset,
                               long xsize, long ysize, DWORD dwRop) {
  if (NULL == szImageBuf || 0 == nImageBufSize) return SMT_ERR_INVALID_PARAM;

  CxImage tmpImage;
  tmpImage.Decode((BYTE *)szImageBuf, nImageBufSize, lCodeType);

  HDC hDC = PrepareDC();
  tmpImage.Stretch(hDC, xoffset, yoffset, xsize, ysize, dwRop);
  EndDC();

  return SMT_ERR_NONE;
}

long RenderBuffer::Save2Image(const char *szFilePath, bool bBgTransparent) {
  long lRtn = Save2Image(m_hPaintBuf, szFilePath, bBgTransparent);
  return lRtn;
}

long RenderBuffer::Save2ImageBuf(char *&szImageBuf, long &lImageBufSize,
                                 long lCodeType, bool bBgTransparent) {
  long lRtn = Save2ImageBuf(m_hPaintBuf, szImageBuf, lImageBufSize, lCodeType,
                            bBgTransparent);
  return lRtn;
}

//////////////////////////////////////////////////////////////////////////
long RenderBuffer::Save2Image(HBITMAP hBitMap, const char *szFilePath,
                              bool bBgTransparent) {
  if (hBitMap == NULL || strlen(szFilePath) == 0) return SMT_ERR_FAILURE;

  CxImage image;

  if (image.CreateFromHBITMAP(hBitMap)) {
    if (bBgTransparent) {
      COLORREF bgClr =
          RGB(255, 255, 255) /*(COLORREF)::GetSysColor(COLOR_WINDOW)*/;
      RGBQUAD transClr;
      transClr.rgbRed = GetRValue(bgClr);
      transClr.rgbGreen = GetGValue(bgClr);
      transClr.rgbBlue = GetBValue(bgClr);
      transClr.rgbReserved = 0;

      image.SetTransIndex(0);
      image.SetTransColor(transClr);
    }

    if (image.Save(szFilePath, GetImageTypeByFileExt(szFilePath))) {
      return SMT_ERR_NONE;
    }
  }

  return SMT_ERR_FAILURE;
}

long RenderBuffer::Save2ImageBuf(HBITMAP hBitMap, char *&szImageBuf,
                                 long &lImageBufSize, long lCodeType,
                                 bool bBgTransparent) {
  if (hBitMap == NULL || szImageBuf != NULL) return SMT_ERR_FAILURE;

  BYTE *pImageBuf = NULL;
  long lSize = 0;
  CxImage image;

  if (image.CreateFromHBITMAP(hBitMap)) {
    if (bBgTransparent) {
      COLORREF bgClr = RGB(255, 255, 255) RGBQUAD transClr;
      transClr.rgbRed = GetRValue(bgClr);
      transClr.rgbGreen = GetGValue(bgClr);
      transClr.rgbBlue = GetBValue(bgClr);
      transClr.rgbReserved = 0;

      image.SetTransIndex(0);
      image.SetTransColor(transClr);
    }

    if (image.Encode(pImageBuf, lSize, lCodeType)) {
      szImageBuf = (char *)pImageBuf;
      lImageBufSize = lSize;

      return SMT_ERR_NONE;
    }

    return SMT_ERR_NONE;
  }

  return SMT_ERR_FAILURE;
}

long RenderBuffer::FreeImageBuf(char *&szImageBuf) {
  SMT_SAFE_DELETE_A(szImageBuf);

  return SMT_ERR_NONE;
}

}  // namespace gfx2d