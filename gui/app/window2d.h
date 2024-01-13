// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_APP_WINDOW2D_H
#define GUI_APP_WINDOW2D_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif  // WIN32_LEAN_AND_MEAN

#include <atlapp.h>
#include <atlbase.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlwin.h>

#include "gfx/2d/renderer/renderer.h"

extern CAppModule _Module;

class Window2D : public CWindowImpl<Window2D>,
                 public CMessageFilter,
                 public CIdleHandler {
 public:
  DECLARE_WND_CLASS(TEXT("SmartGIS"));

 private:
  virtual BOOL PreTranslateMessage(MSG* pMsg) { return FALSE; }

  virtual BOOL OnIdle() { return FALSE; }

  BEGIN_MSG_MAP_EX(Window2D)
  MSG_WM_SIZE(OnSize)
  MSG_WM_CREATE(OnCreate)
  MSG_WM_DESTROY(OnDestroy)
  MSG_WM_PAINT(OnPaint)
  MSG_WM_MOUSEMOVE(OnMouseMove)
  MSG_WM_MOUSEWHEEL(OnMouseWheel)
  MSG_WM_LBUTTONDOWN(OnLButtonDown)
  MSG_WM_LBUTTONUP(OnLButtonUp)
  MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
  MSG_WM_RBUTTONDOWN(OnRButtonDown)
  MSG_WM_RBUTTONUP(OnRButtonUp)
  MSG_WM_RBUTTONDBLCLK(OnRButtonDblClk)
  MSG_WM_COMMAND(OnCommand)
  MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
  MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
#if (_WIN32_WINNT >= 0x0400)
  MSG_WM_MOUSEHOVER(OnMouseHover)
  MSG_WM_MOUSELEAVE(OnMouseLeave)
#endif /* _WIN32_WINNT >= 0x0400 */
  END_MSG_MAP()

  void OnSize(UINT nType, CSize size) {
    if (render_device_) {
      gfx2d::DRect rect;
      rect.x = rect.y = 0;
      rect.width = size.cx;
      rect.height = size.cy;
      render_device_->Resize(rect);

      gfx2d::RenderOptions options;
      render_device_->SetRenderOptions(options);
      render_device_->RefreshDirectly(rect);
    }
  }
  void OnPaint(HDC /*hDC*/) {
    render_device_->BeginRender(gfx2d::RenderDevice::RB_DIRECT);
    render_device_->DrawPolygon(&polygon_);
    render_device_->EndRender(gfx2d::RenderDevice::RB_DIRECT);

    render_device_->Render();
  }

  LRESULT OnCreate(LPCREATESTRUCT lpcs) {
    CMessageLoop* loop = _Module.GetMessageLoop();
    loop->AddMessageFilter(this);
    loop->AddIdleHandler(this);

    polygon_.importFromWkt(&(wkt_.data()));
    renderer_ = new gfx2d::Renderer(::GetModuleHandle(NULL));
    if (ERR_NONE == renderer_->CreateDevice(L"render_device_gdi")) {
      render_device_ = renderer_->GetDevice();
      render_device_->Init(m_hWnd);
      render_device_->SetMapMode(MM_TEXT);
    }

    return S_OK;
  }

  void OnDestroy() {
    PostQuitMessage(0);
    renderer_->ReleaseDevice();
    delete renderer_;
  }

  void OnMouseMove(UINT nFlags, CPoint point) {}
  BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) { return TRUE; }
  void OnLButtonDown(UINT nFlags, CPoint point) {}
  void OnLButtonUp(UINT nFlags, CPoint point) {}
  void OnLButtonDblClk(UINT nFlags, CPoint point) {}
  void OnRButtonDown(UINT nFlags, CPoint point) {}
  void OnRButtonUp(UINT nFlags, CPoint point) {}
  void OnRButtonDblClk(UINT nFlags, CPoint point) {}

  void OnContextMenu(CWindow wnd, CPoint point) {}
  void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl) {}

  void OnEnterSizeMove() {}
  void OnExitSizeMove() {}
#if (_WIN32_WINNT >= 0x0400)
  void OnMouseHover(WPARAM wParam, CPoint ptPos) {}
  void OnMouseLeave() {}
#endif /* _WIN32_WINNT >= 0x0400 */

  std::string wkt_{"POLYGON((0 0, 10 0, 10 10, 0 10, 0 0))"};
  OGRPolygon polygon_;
  gfx2d::Renderer* renderer_{nullptr};
  H2DRENDERDEVICE render_device_{nullptr};
};

#endif  // GUI_APP_WINDOW2D_H
