// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_BROWSER_FRAME_H_
#define GUI_BROWSER_FRAME_H_

#include "gui/app.h"
#include "gui/view.h"

namespace gui {
class Frame : public CFrameWindowImpl<Frame>,
              public CUpdateUI<Frame>,
              public CMessageFilter,
              public CIdleHandler {
 public:
  Frame();
  virtual ~Frame();

 public:
  DECLARE_FRAME_WND_CLASS(L"Frame", IDR_MAINFRAME)

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

  BEGIN_MSG_MAP(Frame)
  MESSAGE_HANDLER(WM_CREATE, OnCreate)
  MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
  MESSAGE_HANDLER(WM_CLOSE, OnClose)
  MESSAGE_HANDLER(WM_ENTERSIZEMOVE, OnEnterSizeMove)
  MESSAGE_HANDLER(WM_EXITSIZEMOVE, OnExitSizeMove)
  COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
  COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
  COMMAND_ID_HANDLER(ID_FILE_SAVE_AS, OnFileSaveAs)
  COMMAND_ID_HANDLER(ID_FILE_PRINT, OnPrint)
  COMMAND_ID_HANDLER(ID_FILE_PRINT_PREVIEW, OnPrintPreview)
  COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
  COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
  COMMAND_ID_HANDLER(ID_NEWWINDOW_DEFAULT, OpenDefaultWindow)
  COMMAND_ID_HANDLER(ID_FEATURE_SWITCH, OnSwitch)
  COMMAND_ID_HANDLER(ID_FEATURE_SHOW, OnShow)
  COMMAND_ID_HANDLER(ID_FEATURE_HIDE, OnHide)
  COMMAND_ID_HANDLER(ID_FEATURE_REFRESH, OnRefresh)
  COMMAND_ID_HANDLER(ID_FEATURE_ZOOMIN, OnZoomin)
  COMMAND_ID_HANDLER(ID_FEATURE_ZOOMOUT, OnZoomout)
  REFLECT_NOTIFICATIONS()
  CHAIN_MSG_MAP(CUpdateUI<Frame>)
  CHAIN_MSG_MAP(CFrameWindowImpl<Frame>)
  END_MSG_MAP()

  BEGIN_UPDATE_UI_MAP(Frame)
  UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
  UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
  END_UPDATE_UI_MAP()

 public:
  LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                   BOOL& /*bHandled*/);
  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                    BOOL& /*bHandled*/);
  LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                  BOOL& /*bHandled*/);

  LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                     BOOL& /*bHandled*/);

  LRESULT OnEnterSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                          BOOL& bHandled);
  LRESULT OnExitSizeMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                         BOOL& bHandled);

  LRESULT OnShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                 BOOL& /*bHandled*/);
  LRESULT OnHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                 BOOL& /*bHandled*/);
  LRESULT OnSwitch(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                   BOOL& /*bHandled*/);
  LRESULT OnGoBack(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                   BOOL& /*bHandled*/);
  LRESULT OnGoForward(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                      BOOL& /*bHandled*/);
  LRESULT OnFileSaveAs(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                       BOOL& /*bHandled*/);
  LRESULT OnViewFullscreen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                           BOOL& /*bHandled*/);
  LRESULT OnViewToolBar(WORD /*wNotifyCode*/ WORD /*wID*/, HWND /*hWndCtl*/,
                        BOOL& /*bHandled*/);
  LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                          BOOL& /*bHandled*/);
  LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                     BOOL& /*bHandled*/);
  LRESULT OnPrint(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                  BOOL& /*bHandled*/);
  LRESULT OnPrintPreview(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                         BOOL& /*bHandled*/);

  LRESULT OpenDefaultWindow(WORD /*wNotifyCode*/, WORD /*wID*/,
                            HWND /*hWndCtl*/, BOOL& /*bHandled*/);
  LRESULT OnRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                    BOOL& /*bHandled*/);
  LRESULT OnZoomin(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                   BOOL& /*bHandled*/);
  LRESULT OnZoomout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/,
                    BOOL& /*bHandled*/);

  void UpdateLayout(BOOL bResizeBars = TRUE);

 public:
  void Show();
  void Hide();
  void Resize();
  void Resize(const RECT& rect);

 private:
  View view_;

 private:
  uint32 zoom_level_;
};
}  // namespace gui
#endif  // GUI_BROWSER_FRAME_H_
