// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/app/window2d.h"

BOOL Window2D::PreTranslateMessage(MSG* pMsg) { return FALSE; }

BOOL Window2D::OnIdle() { return FALSE; }

void Window2D::OnSize(UINT nType, CSize size) {
  if (render_device_) {
    gfx2d::DRect rect;
    rect.x = rect.y = 0;
    rect.width = size.cx;
    rect.height = size.cy;
    render_device_->Resize(rect);

    gfx2d::RenderOptions options;
    render_device_->SetRenderOptions(options);
    render_device_->BeginRender(gfx2d::RenderDevice::RB_COMPOSIT);
    render_device_->DrawPolygon(&polygon_);
    render_device_->DrawAnno(&text_anchor_, text_.c_str(), 0.f, 20, 20, 3);
    render_device_->EndRender(gfx2d::RenderDevice::RB_COMPOSIT);
    render_device_->RefreshDirectly(rect);
  }
}
void Window2D::OnPaint(HDC /*hDC*/) { render_device_->Render(); }

LRESULT Window2D::OnCreate(LPCREATESTRUCT lpcs) {
  CMessageLoop* loop = _Module.GetMessageLoop();
  loop->AddMessageFilter(this);
  loop->AddIdleHandler(this);

  {
    auto* buffer = polygon_wkt_.data();
    polygon_.importFromWkt(&buffer);
  }

  {
    auto* buffer = text_anchor_wkt_.data();
    text_anchor_.importFromWkt(&buffer);
  }

  renderer_ = new gfx2d::Renderer(::GetModuleHandle(NULL));
  if (ERR_NONE == renderer_->CreateDevice(L"render_device_gdi")) {
    render_device_ = renderer_->GetDevice();
    render_device_->Init(m_hWnd);
    render_device_->SetMapMode(MM_TEXT);

    gfx2d::LRect rect;
    rect.x = rect.y = 400;
    rect.width = 800;
    rect.height = 800;
    render_device_->ZoomToRect(rect);
  }

  return S_OK;
}

void Window2D::OnDestroy() {
  PostQuitMessage(0);
  renderer_->ReleaseDevice();
  delete renderer_;
}

void Window2D::OnMouseMove(UINT nFlags, CPoint point) {}
BOOL Window2D::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
  return TRUE;
}
void Window2D::OnLButtonDown(UINT nFlags, CPoint point) {}
void Window2D::OnLButtonUp(UINT nFlags, CPoint point) {}
void Window2D::OnLButtonDblClk(UINT nFlags, CPoint point) {}
void Window2D::OnRButtonDown(UINT nFlags, CPoint point) {}
void Window2D::OnRButtonUp(UINT nFlags, CPoint point) {}
void Window2D::OnRButtonDblClk(UINT nFlags, CPoint point) {}

void Window2D::OnContextMenu(CWindow wnd, CPoint point) {}
void Window2D::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl) {}

void Window2D::OnEnterSizeMove() {}
void Window2D::OnExitSizeMove() {}
#if (_WIN32_WINNT >= 0x0400)
void Window2D::OnMouseHover(WPARAM wParam, CPoint ptPos) {}
void Window2D::OnMouseLeave() {}
#endif /* _WIN32_WINNT >= 0x0400 */