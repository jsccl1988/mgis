// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef SUI_APP_WINDOW_H
#define SUI_APP_WINDOW_H

#define WIN32_LEAN_AND_MEAN

#include <atlbase.h>

#include <atlapp.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <atlwin.h>

extern CAppModule _Module;

class Window : public CWindowImpl<Window>,
               public CMessageFilter,
               public CIdleHandler {
 public:
  DECLARE_WND_CLASS(TEXT("SmartGIS"));

 private:
  virtual BOOL PreTranslateMessage(MSG* pMsg) { return FALSE; }

  virtual BOOL OnIdle() { return FALSE; }

  BEGIN_MSG_MAP_EX(Window)
  MSG_WM_PAINT(OnPaint)
  MSG_WM_CREATE(OnCreate)
  MSG_WM_DESTROY(OnDestroy)
  END_MSG_MAP()

  void OnPaint(HDC /*hDC*/) {
    CPaintDC dc(m_hWnd);
    CRect rect;
    GetClientRect(rect);
    dc.DrawText(_T("Hello, SmartGIS"), -1, rect,
                DT_SINGLELINE | DT_CENTER | DT_VCENTER);
  }

  LRESULT OnCreate(LPCREATESTRUCT lpcs) {
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);
    return 0;
  }

  void OnDestroy() { PostQuitMessage(0); }
};

#endif  // SUI_APP_WINDOW_H_
