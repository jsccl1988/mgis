// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "content/control/window/map_window.h"

#define ToDPoint(point) gfx2d::DPoint(point.x, point.y)

namespace content {
static const UINT kRefreshTimer = 69;
static const UINT kNotifyTimer = 70;

LRESULT MapWindow::OnCreate(LPCREATESTRUCT lpcs) {
  auto* environment = content::Environment::GetSingletonPtr();
  // message loop
  CMessageLoop* loop = _Module.GetMessageLoop();
  if (loop) {
    loop->AddMessageFilter(this);
    loop->AddIdleHandler(this);
  }

  // map
  {}

  // renderer
  renderer_ = new gfx2d::Renderer(::GetModuleHandle(NULL));
  if (ERR_NONE == renderer_->CreateDevice(L"render_device_gdi")) {
    render_device_ = renderer_->GetDevice();
    render_device_->Init(m_hWnd);
    render_device_->SetMapMode(MM_TEXT);

    gfx2d::LRect lrect;
    lrect.x = lrect.y = 0;
    lrect.width = 3840;
    lrect.height = 2400;

    gfx2d::DRect drect;
    drect.x = drect.y = 0;
    drect.width = 3840;
    drect.height = 2400;

    render_device_->Resize(drect);
    render_device_->ZoomToRect(lrect);
  }

  // tool
  content::ToolFactory::CreateTool(navigate_tool_,
                                   content::ToolFactory::Navigate);

  if (ERR_NONE != navigate_tool_->Init(m_hWnd, render_device_)) {
    return S_FALSE;
  }
  navigate_tool_->SetActive();

  // navigate menu
  m_hMainMenu = ::CreatePopupMenu();
  m_hContexMenu = ::CreatePopupMenu();

  HMENU hMenu =
      content::CreateListenerMenu(navigate_tool_, content::FIG_2DMFMENU);
  if (GetMenuItemCount(hMenu) > 0)
    AppendMenu(m_hMainMenu, MF_POPUP, (UINT)hMenu, navigate_tool_->GetName());

  content::AppendListenerMenu(m_hContexMenu, navigate_tool_,
                              content::FIG_2DVIEW, false);

  // timer
  auto system_options = environment->GetSystemOptions();
  m_uiRefreshTimer = ::SetTimer(m_hWnd, kRefreshTimer,
                                system_options.view2d_refresh_elapse, 0);
  m_uiNotifyTimer =
      ::SetTimer(m_hWnd, kNotifyTimer, system_options.view2d_notify_elapse, 0);

  return S_OK;
}

void MapWindow::OnDestroy() {
  ::PostQuitMessage(0);
  ::KillTimer(m_hWnd, kRefreshTimer);
  ::KillTimer(m_hWnd, kNotifyTimer);

  content::ToolFactory::DestoryTool(navigate_tool_);

  ::DestroyMenu(m_hMainMenu);
  ::DestroyMenu(m_hContexMenu);

  renderer_->ReleaseDevice();
  delete renderer_;
}

void MapWindow::OnTimer(UINT_PTR nIDEvent) {
  switch (nIDEvent) {
    case kRefreshTimer: {
      auto* tool_manager = content::ToolManager::GetSingletonPtr();
      auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
      if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
        tool->Timer();
      }
    } break;
    case kNotifyTimer: {
      if (render_device_) {
        auto* environment = content::Environment::GetSingletonPtr();
        auto system_options = environment->GetSystemOptions();
        gfx2d::RenderOptions options;
        options.show_mbr = system_options.show_mbr;
        options.show_point = system_options.show_point;
        options.point_radius = system_options.point_radius;

        render_device_->SetRenderOptions(options);
        render_device_->Timer();

        ::PostMessage(m_hWnd, WM_PAINT, NULL, NULL);
      }
    } break;
    default:
      break;
  }
}

void MapWindow::OnSize(UINT nType, CSize size) {
  auto* environment = content::Environment::GetSingletonPtr();
  if (render_device_) {
    gfx2d::DRect rect;
    rect.x = rect.y = 0;
    rect.width = size.cx;
    rect.height = size.cy;
    render_device_->Resize(rect);

    auto system_options = environment->GetSystemOptions();
    gfx2d::RenderOptions options;
    options.show_mbr = system_options.show_mbr;
    options.show_point = system_options.show_point;
    options.point_radius = system_options.point_radius;

    render_device_->SetRenderOptions(options);
    render_device_->BeginRender(gfx2d::RenderDevice::RB_COMPOSIT);
    render_device_->RenderDebug();
    render_device_->EndRender(gfx2d::RenderDevice::RB_COMPOSIT);
    render_device_->RefreshDirectly(rect);
  }
}
void MapWindow::OnPaint(HDC /*hDC*/) {
  render_device_->Render();

  auto* environment = content::Environment::GetSingletonPtr();
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  content::Tool* tool =
      dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->AuxDraw();
  }
}

void MapWindow::OnMouseMove(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseMove(nFlags, ToDPoint(point));
  }
}
BOOL MapWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseWheel(nFlags, zDelta, ToDPoint(point));
  }
  return TRUE;
}
void MapWindow::OnLButtonDown(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDown(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonUp(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonUp(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonDown(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDown(nFlags, ToDPoint(point));

    if (tool->IsEnableContexMenu()) {
      CMenu contex_menu;
      contex_menu.Attach(m_hContexMenu);
      contex_menu.TrackPopupMenu(
          TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
          m_hWnd);
      contex_menu.Detach();
    }
  }
}
void MapWindow::OnRButtonUp(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonUp(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonDblClk(UINT nFlags, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyDown(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyUp(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnContextMenu(CWindow wnd, CPoint point) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    if (tool->IsEnableContexMenu()) {
      CMenu contex_menu;
      contex_menu.Attach(m_hContexMenu);
      contex_menu.TrackPopupMenu(
          TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, point.x, point.y,
          m_hWnd);
      contex_menu.Detach();
    }
  }
}
void MapWindow::OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl) {
  content::MessageListener::Message message;
  message.id = nID;
  message.source_window = m_hWnd;

  if (nID >= MESSAGE_CMD_BEGIN && nID <= MESSAGE_CMD_END) {
    auto* tool_manager = content::ToolManager::GetSingletonPtr();
    tool_manager->Notify(TOOL_BROADCAST, message);
  }
}

void MapWindow::OnEnterSizeMove() {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnExitSizeMove() {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#if (_WIN32_WINNT >= 0x0400)
void MapWindow::OnMouseHover(WPARAM wParam, CPoint ptPos) {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnMouseLeave() {
  auto* tool_manager = content::ToolManager::GetSingletonPtr();
  auto* tool = dynamic_cast<content::Tool*>(tool_manager->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#endif /* _WIN32_WINNT >= 0x0400 */
}  // namespace content
