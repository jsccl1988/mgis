// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#include "gui/frame.h"

#include <assert.h>

#include <fstream>
#include <string>

#include "gui/map_render/map_render_manager.h"
#include "gui/base/path/base_paths.h"

namespace gui {
Frame::Frame() : zoom_level_(100) {}
Frame::~Frame() {}

BOOL Frame::PreTranslateMessage(MSG* pMsg) {
  if (CFrameWindowImpl<Frame>::PreTranslateMessage(pMsg)) return TRUE;

  return browser_view_.PreTranslateMessage(pMsg);
}

BOOL Frame::OnIdle() {
  UIUpdateToolBar();
  return FALSE;
}

LRESULT Frame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                        BOOL& bHandled) {
  UIAddToolBar(CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE,
                                       ATL_SIMPLE_TOOLBAR_PANE_STYLE));

  CreateSimpleStatusBar();

  // browser view
  view_.frame_ = this;
  m_hWndClient =
      view_.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_MAXIMIZE,
                   WS_EX_STATICEDGE);
  return 0;
}

LRESULT Frame::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam,
                       BOOL& bHandled) {
  DestroyWindow();

  return 1;
}

BOOL CALLBACK _EnumChildProc(HWND hWnd, LPARAM lParam) {
  if (hWnd) {
    ::ShowWindow(hWnd, SW_HIDE);
    ::SetParent(hWnd, NULL);
    ::ShowWindow(hWnd, SW_HIDE);
  }

  return TRUE;
}

LRESULT Frame::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam,
                         BOOL& bHandled) {
  EnumChildWindows(m_hWndClient, _EnumChildProc, NULL);
  view_.DestroyWindow();
  MapContentManager::Instance()->QuitMessageLoop();

  return 0;
}

LRESULT Frame::OnEnterSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/,
                               LPARAM /*lParam*/, BOOL& bHandled) {
  return 0;
}

LRESULT Frame::OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/,
                              LPARAM /*lParam*/, BOOL& bHandled) {
  return 0;
}

LRESULT Frame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                          BOOL& /*bHandled*/) {
  PostMessage(WM_CLOSE);

  return 0;
}

LRESULT Frame::OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/,
                            HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/,
                             HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  static BOOL bVisible = TRUE;
  bVisible = !bVisible;
  CReBarCtrl rebar = m_hWndToolBar;
  int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1);
  rebar.ShowBand(nBandIndex, bVisible);
  UISetCheck(ID_VIEW_TOOLBAR, bVisible);
  UpdateLayout();

  return 0;
}

LRESULT Frame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/,
                               HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
  ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
  UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
  UpdateLayout();

  return 0;
}

void Frame::UpdateLayout(BOOL bResizeBars) {
  RECT rect = {0};
  GetClientRect(&rect);

  // position bars and offset their dimensions
  UpdateBarsPosition(rect, bResizeBars);

  // resize client window
  if (m_hWndClient != NULL)
    ::SetWindowPos(m_hWndClient, NULL, rect.left, rect.top,
                   rect.right - rect.left, rect.bottom - rect.top,
                   SWP_NOZORDER | SWP_NOACTIVATE);
}

LRESULT Frame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                          BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                       BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnPrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/,
                              HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OpenDefaultWindow(WORD /*wNotifyCode*/, WORD /*wID*/,
                                 HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  Frame* frame = new Frame(content::WB_TRIDENT);
  browser_frame->CreateEx();
  browser_frame->ShowWindow(SW_SHOWNORMAL);
  browser_frame->Show();

  return 0;
}

LRESULT Frame::OnShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                         BOOL& /*bHandled*/) {
  return 0;
}

LRESULT Frame::OnZoomin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                        BOOL& /*bHandled*/) {
  if (zoom_level_ < 20) zoom_level_ = 20;
  if (zoom_level_ > 400) zoom_level_ = 400;

  zoom_level_ += 10;

  return 0;
}

LRESULT Frame::OnZoomout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                         BOOL& /*bHandled*/) {
  if (zoom_level_ < 20) zoom_level_ = 20;
  if (zoom_level_ > 400) zoom_level_ = 400;

  zoom_level_ -= 10;

  return 0;
}

void Frame::Show() {}

void Frame::Hide() {}

void Frame::Resize() {
  RECT rc = {0, 0, 0, 0};
  ::GetWindowRect(m_hWndClient, &rc);
  base::Rect rect;
  rect.x = rc.left;
  rect.y = rc.top;
  rect.width = (rc.right - rc.left);
  rect.height = (rc.bottom - rc.top);

  if (rect.width == 0 && rect.height == 0) return;
}

void Frame::Resize(const RECT& rect) {
  base::Rect rc;
  rc.x = rect.left;
  rc.y = rect.top;
  rc.width = (rect.right - rect.left);
  rc.height = (rect.bottom - rect.top);

  if (rc.width == 0 && rc.height == 0) return;

  ::SetWindowPos(m_hWnd, NULL, rc.x, rc.y, rc.width, rc.height,
                 SWP_NOZORDER | SWP_NOACTIVATE);

  UpdateLayout();
}
}  // namespace gui
