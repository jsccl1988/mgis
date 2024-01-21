// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef CONTENT_CONTROL_MAP_BOX_MAP_WINDOW_H
#define CONTENT_CONTROL_MAP_BOX_MAP_WINDOW_H

#include "content/common/environment.h"
#include "content/control/map_box/map_app.h"
#include "content/control/tool_factory.h"
#include "gfx/2d/renderer/renderer.h"

namespace content {
class MapWindow : public CWindowImpl<MapWindow>,
                  public CMessageFilter,
                  public CIdleHandler {
 public:
  DECLARE_WND_CLASS(TEXT("SmartGIS"));

 private:
  BEGIN_MSG_MAP_EX(MapWindow)
  MSG_WM_SIZE(OnSize)
  MSG_WM_CREATE(OnCreate)
  MSG_WM_DESTROY(OnDestroy)
  MSG_WM_TIMER(OnTimer)
  MSG_WM_PAINT(OnPaint)
  MSG_WM_MOUSEMOVE(OnMouseMove)
  MSG_WM_MOUSEWHEEL(OnMouseWheel)
  MSG_WM_LBUTTONDOWN(OnLButtonDown)
  MSG_WM_LBUTTONUP(OnLButtonUp)
  MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk)
  MSG_WM_RBUTTONDOWN(OnRButtonDown)
  MSG_WM_RBUTTONUP(OnRButtonUp)
  MSG_WM_RBUTTONDBLCLK(OnRButtonDblClk)
  MSG_WM_KEYDOWN(OnKeyDown)
  MSG_WM_KEYUP(OnKeyUp)
  MSG_WM_CONTEXTMENU(OnContextMenu)
  MSG_WM_COMMAND(OnCommand)
  MSG_WM_ENTERSIZEMOVE(OnEnterSizeMove)
  MSG_WM_EXITSIZEMOVE(OnExitSizeMove)
#if (_WIN32_WINNT >= 0x0400)
  MSG_WM_MOUSEHOVER(OnMouseHover)
  MSG_WM_MOUSELEAVE(OnMouseLeave)
#endif /* _WIN32_WINNT >= 0x0400 */
  END_MSG_MAP()

  virtual BOOL PreTranslateMessage(MSG* pMsg) { return FALSE; }
  virtual BOOL OnIdle() { return FALSE; }

  void OnSize(UINT nType, CSize size);
  void OnPaint(HDC /*hDC*/);
  LRESULT OnCreate(LPCREATESTRUCT lpcs);
  void OnDestroy();
  void OnTimer(UINT_PTR nIDEvent);

  void OnMouseMove(UINT nFlags, CPoint point);
  BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint point);
  void OnLButtonDown(UINT nFlags, CPoint point);
  void OnLButtonUp(UINT nFlags, CPoint point);
  void OnLButtonDblClk(UINT nFlags, CPoint point);
  void OnRButtonDown(UINT nFlags, CPoint point);
  void OnRButtonUp(UINT nFlags, CPoint point);
  void OnRButtonDblClk(UINT nFlags, CPoint point);
  void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
  void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

  void OnContextMenu(CWindow wnd, CPoint point);
  void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl);

  void OnEnterSizeMove();
  void OnExitSizeMove();
#if (_WIN32_WINNT >= 0x0400)
  void OnMouseHover(WPARAM wParam, CPoint ptPos);
  void OnMouseLeave();
#endif /* _WIN32_WINNT >= 0x0400 */

  HMENU m_hContexMenu{NULL};
  HMENU m_hMainMenu{NULL};
  UINT m_uiNotifyTimer{0};
  UINT m_uiRefreshTimer{0};

  gfx2d::Renderer* renderer_{nullptr};
  content::Tool* navigate_tool_{nullptr};
  H2DRENDERDEVICE render_device_{nullptr};
};
}  // namespace content

#endif  // CONTENT_CONTROL_MAP_BOX_MAP_WINDOW_H
