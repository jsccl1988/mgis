// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_APP_WINDOW2D_H
#define GUI_APP_WINDOW2D_H

#include "gfx/2d/renderer/renderer.h"
#include "gui/app/app.h"

extern CAppModule _Module;

class Window2D : public CWindowImpl<Window2D>,
                 public CMessageFilter,
                 public CIdleHandler {
 public:
  DECLARE_WND_CLASS(TEXT("SmartGIS"));

 private:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

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

  void OnSize(UINT nType, CSize size);
  void OnPaint(HDC /*hDC*/);
  LRESULT OnCreate(LPCREATESTRUCT lpcs);

  void OnDestroy();

  void OnMouseMove(UINT nFlags, CPoint point);
  BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
  void OnLButtonDown(UINT nFlags, CPoint point);
  void OnLButtonUp(UINT nFlags, CPoint point);
  void OnLButtonDblClk(UINT nFlags, CPoint point);
  void OnRButtonDown(UINT nFlags, CPoint point);
  void OnRButtonUp(UINT nFlags, CPoint point);
  void OnRButtonDblClk(UINT nFlags, CPoint point);

  void OnContextMenu(CWindow wnd, CPoint point);
  void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);

  void OnEnterSizeMove();
  void OnExitSizeMove();
#if (_WIN32_WINNT >= 0x0400)
  void OnMouseHover(WPARAM wParam, CPoint ptPos);
  void OnMouseLeave();
#endif /* _WIN32_WINNT >= 0x0400 */

  std::string polygon_wkt_{
      "POLYGON((500 500, 1000 500, 1000 1000, 500 1000, 500 500))"};
  std::string text_anchor_wkt_{"POINT(750 750)"};

  OGRPolygon polygon_;
  OGRPoint text_anchor_;
  std::string text_{"smart gis"};
  gfx2d::Renderer* renderer_{nullptr};
  H2DRENDERDEVICE render_device_{nullptr};
};

#endif  // GUI_APP_WINDOW2D_H
