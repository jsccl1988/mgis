// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "content/control/window/map_window.h"

#include "gfx/2d/renderer/style.h"

#define ToDPoint(point) gfx2d::DPoint(point.x, point.y)

namespace content {
static const UINT kRefreshTimer = 69;
static const UINT kNotifyTimer = 70;

LRESULT MapWindow::OnCreate(LPCREATESTRUCT lpcs) {
  auto& environment = content::Environment::GetInstance();
  // message loop
  CMessageLoop* loop = _Module.GetMessageLoop();
  if (loop) {
    loop->AddMessageFilter(this);
    loop->AddIdleHandler(this);
  }

  // style
  {
    auto& style_manager = gfx2d::StyleManager::GetInstance();
    auto& style_options = environment.get()->GetSystemOptions().style_options;
    auto& flash_options = environment.get()->GetSystemOptions().flash_options;

    gfx2d::PenDesc pen_desc;
    gfx2d::BrushDesc brush_desc;
    gfx2d::AnnotationDesc anno_desc;
    gfx2d::SymbolDesc symbol_desc;

    style_manager.get()->SetDefaultStyle("Default", pen_desc, brush_desc,
                                         anno_desc, symbol_desc);

    auto* style1 = style_manager.get()->CreateStyle(
        style_options.point_style.c_str(), pen_desc, brush_desc, anno_desc,
        symbol_desc);
    auto* style2 = style_manager.get()->CreateStyle(
        style_options.curve_style.c_str(), pen_desc, brush_desc, anno_desc,
        symbol_desc);
    auto* style3 = style_manager.get()->CreateStyle(
        style_options.surface_style.c_str(), pen_desc, brush_desc, anno_desc,
        symbol_desc);
    auto* style4 = style_manager.get()->CreateStyle(
        style_options.aux_style.c_str(), pen_desc, brush_desc, anno_desc,
        symbol_desc);

    auto* style5 = style_manager.get()->CreateStyle(
        style_options.point_flash_style1.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);
    auto* style6 = style_manager.get()->CreateStyle(
        style_options.point_flash_style2.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);

    auto* style7 = style_manager.get()->CreateStyle(
        style_options.curve_flash_style1.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);
    auto* style8 = style_manager.get()->CreateStyle(
        style_options.curve_flash_style2.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);

    auto* style9 = style_manager.get()->CreateStyle(
        style_options.surface_flash_style1.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);
    auto* style10 = style_manager.get()->CreateStyle(
        style_options.surface_flash_style2.c_str(), pen_desc, brush_desc,
        anno_desc, symbol_desc);

    //////////////////////////////////////////////////////////////////////////
    style1->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                         gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);
    style2->SetStyleType(gfx2d::ST_PenDesc);
    style3->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);
    style4->SetStyleType(gfx2d::ST_PenDesc);

    style5->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                         gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);
    style6->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc |
                         gfx2d::ST_AnnoDesc | gfx2d::ST_SymbolDesc);

    style7->SetStyleType(gfx2d::ST_PenDesc);
    style8->SetStyleType(gfx2d::ST_PenDesc);

    style9->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);
    style10->SetStyleType(gfx2d::ST_PenDesc | gfx2d::ST_BrushDesc);

    //////////////////////////////////////////////////////////////////////////
    // 1
    pen_desc.color = RGB(255, 0, 0);
    style1->SetPenDesc(pen_desc);

    brush_desc.color = RGB(0, 255, 255);
    style1->SetBrushDesc(brush_desc);

    symbol_desc.id = 0;
    symbol_desc.width = symbol_desc.height = 1.6;
    style1->SetSymbolDesc(symbol_desc);

    //////////////////////////////////////////////////////////////////////////
    // 2
    pen_desc.color = RGB(0, 0, 255);
    style2->SetPenDesc(pen_desc);

    //////////////////////////////////////////////////////////////////////////
    // 3
    pen_desc.width = 0.001;
    pen_desc.color = RGB(255, 0, 0);
    style3->SetPenDesc(pen_desc);

    // brush_desc.brushTp = SmtBrushDesc::BT_Hatch;
    brush_desc.color = RGB(77, 255, 0);
    style3->SetBrushDesc(brush_desc);

    //////////////////////////////////////////////////////////////////////////
    // 4
    pen_desc.color = RGB(255, 0, 0);
    style4->SetPenDesc(pen_desc);

    //////////////////////////////////////////////////////////////////////////
    // 5
    pen_desc.color = flash_options.color1;
    pen_desc.width = 0.002;
    style5->SetPenDesc(pen_desc);

    brush_desc.color = flash_options.color2;
    style5->SetBrushDesc(brush_desc);

    anno_desc.color = flash_options.color1;
    style5->SetAnnoDesc(anno_desc);

    //////////////////////////////////////////////////////////////////////////
    // 6
    pen_desc.color = flash_options.color2;
    pen_desc.width = 0.002;
    style6->SetPenDesc(pen_desc);

    brush_desc.color = flash_options.color1;
    style6->SetBrushDesc(brush_desc);

    anno_desc.color = flash_options.color2;
    style6->SetAnnoDesc(anno_desc);

    //////////////////////////////////////////////////////////////////////////
    // 7
    pen_desc.color = flash_options.color1;
    pen_desc.width = 0.002;
    style7->SetPenDesc(pen_desc);

    //////////////////////////////////////////////////////////////////////////
    // 8
    pen_desc.color = flash_options.color2;
    pen_desc.width = 0.002;
    style8->SetPenDesc(pen_desc);

    //////////////////////////////////////////////////////////////////////////
    // 9
    pen_desc.color = flash_options.color1;
    pen_desc.width = 0.002;
    style9->SetPenDesc(pen_desc);

    brush_desc.color = flash_options.color2;
    style9->SetBrushDesc(brush_desc);

    //////////////////////////////////////////////////////////////////////////
    // 10
    pen_desc.color = flash_options.color2;
    pen_desc.width = 0.002;
    style10->SetPenDesc(pen_desc);

    brush_desc.color = flash_options.color1;
    style10->SetBrushDesc(brush_desc);
  }

  // map
  {}

  // renderer
  {
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
  }

  // tool
  {
    auto& style_manager = gfx2d::StyleManager::GetInstance();
    auto& style_options = environment.get()->GetSystemOptions().style_options;
    content::ToolFactory::CreateTool(navigate_tool_,
                                     content::ToolFactory::Navigate);
    content::ToolFactory::CreateTool(select_tool_,
                                     content::ToolFactory::Select);
    content::ToolFactory::CreateTool(flash_tool_, content::ToolFactory::Flash);
    content::ToolFactory::CreateTool(edit_tool_, content::ToolFactory::Edit);

    navigate_tool_->SetToolStyleName(style_options.aux_style.c_str());
    if (ERR_NONE != navigate_tool_->Init(m_hWnd, render_device_)) {
      return S_FALSE;
    }

    select_tool_->SetToolStyleName(style_options.aux_style.c_str());
    if (ERR_NONE != select_tool_->Init(m_hWnd, render_device_)) {
      return S_FALSE;
    }

    flash_tool_->SetToolStyleName(style_options.aux_style.c_str());
    if (ERR_NONE != flash_tool_->Init(m_hWnd, render_device_)) {
      return S_FALSE;
    }

    edit_tool_->SetToolStyleName(style_options.aux_style.c_str());
    if (ERR_NONE != edit_tool_->Init(m_hWnd, render_device_)) {
      return S_FALSE;
    }

    navigate_tool_->SetActive();
  }

  // navigate menu
  {
    m_hMainMenu = ::CreatePopupMenu();
    m_hContexMenu = ::CreatePopupMenu();

    HMENU hMenu =
        content::CreateListenerMenu(navigate_tool_, content::FIG_2DMFMENU);
    if (GetMenuItemCount(hMenu) > 0)
      AppendMenu(m_hMainMenu, MF_POPUP, (UINT)hMenu, navigate_tool_->GetName());

    content::AppendListenerMenu(m_hContexMenu, navigate_tool_,
                                content::FIG_2DVIEW, false);
    content::AppendListenerMenu(m_hContexMenu, select_tool_,
                                content::FIG_2DVIEW, true);
    content::AppendListenerMenu(m_hContexMenu, flash_tool_, content::FIG_2DVIEW,
                                true);
    content::AppendListenerMenu(m_hContexMenu, edit_tool_, content::FIG_2DVIEW,
                                true);
  }

  // timer
  {
    auto system_options = environment.get()->GetSystemOptions();
    m_uiRefreshTimer = ::SetTimer(m_hWnd, kRefreshTimer,
                                  system_options.view2d_refresh_elapse, 0);
    m_uiNotifyTimer = ::SetTimer(m_hWnd, kNotifyTimer,
                                 system_options.view2d_notify_elapse, 0);
  }

  return S_OK;
}

void MapWindow::OnDestroy() {
  ::PostQuitMessage(0);
  ::KillTimer(m_hWnd, kRefreshTimer);
  ::KillTimer(m_hWnd, kNotifyTimer);

  content::ToolFactory::DestoryTool(navigate_tool_);
  content::ToolFactory::DestoryTool(select_tool_);
  content::ToolFactory::DestoryTool(flash_tool_);
  content::ToolFactory::DestoryTool(edit_tool_);

  ::DestroyMenu(m_hMainMenu);
  ::DestroyMenu(m_hContexMenu);

  renderer_->ReleaseDevice();
  delete renderer_;
}

void MapWindow::OnTimer(UINT_PTR event) {
  switch (event) {
    case kRefreshTimer: {
      auto& tool_manager = content::ToolManager::GetInstance();
      auto* tool =
          dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
      if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
        tool->Timer();
      }
    } break;
    case kNotifyTimer: {
      if (render_device_) {
        auto& environment = content::Environment::GetInstance();
        auto system_options = environment.get()->GetSystemOptions();
        gfx2d::RenderOptions options;
        options.show_mbr = system_options.show_mbr;
        options.show_point = system_options.show_point;
        options.point_radius = system_options.point_radius;

        render_device_->SetRenderOptions(options);
      }
    } break;
    default:
      break;
  }
}

void MapWindow::OnSize(UINT nType, CSize size) {
  auto& environment = content::Environment::GetInstance();
  if (render_device_) {
    gfx2d::DRect rect;
    rect.x = rect.y = 0;
    rect.width = size.cx;
    rect.height = size.cy;
    render_device_->Resize(rect);

    auto system_options = environment.get()->GetSystemOptions();
    gfx2d::RenderOptions options;
    options.show_mbr = system_options.show_mbr;
    options.show_point = system_options.show_point;
    options.point_radius = system_options.point_radius;

    render_device_->SetRenderOptions(options);
    render_device_->RefreshDirectly(rect);
  }
}
void MapWindow::OnPaint(HDC /*hDC*/) {
  auto& environment = content::Environment::GetInstance();
  auto& tool_manager = content::ToolManager::GetInstance();
  content::Tool* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());

  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->AuxDraw();
  }

  render_device_->Swap();
}

void MapWindow::OnMouseMove(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseMove(nFlags, ToDPoint(point));
  }
}
BOOL MapWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->MouseWheel(nFlags, zDelta, ToDPoint(point));
  }
  return TRUE;
}
void MapWindow::OnLButtonDown(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDown(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonUp(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonUp(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->LButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonDown(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDown(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnRButtonUp(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonUp(nFlags, ToDPoint(point));

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
void MapWindow::OnRButtonDblClk(UINT nFlags, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->RButtonDClick(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyDown(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    tool->KeyUp(nChar, nRepCnt, nFlags);
  }
}
void MapWindow::OnContextMenu(CWindow wnd, CPoint point) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
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
    auto& tool_manager = content::ToolManager::GetInstance();
    tool_manager.get()->Notify(TOOL_BROADCAST, message);
  }
}

void MapWindow::OnEnterSizeMove() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnExitSizeMove() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#if (_WIN32_WINNT >= 0x0400)
void MapWindow::OnMouseHover(WPARAM wParam, CPoint ptPos) {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
void MapWindow::OnMouseLeave() {
  auto& tool_manager = content::ToolManager::GetInstance();
  auto* tool =
      dynamic_cast<content::Tool*>(tool_manager.get()->GetActiveTool());
  if (tool && (tool->GetOwnerWnd() == m_hWnd)) {
    // tool->MouseMove(nFlags, ToDPoint(point));
  }
}
#endif /* _WIN32_WINNT >= 0x0400 */
}  // namespace content
