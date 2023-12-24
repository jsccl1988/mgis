// Copyright (c) 2023 The MGIS Authors.
// All rights reserved.

#ifndef GUI_VIEW_H_
#define GUI_VIEW_H_

#include "gui/app.h"

namespace gui {

#define NO_ERASEBACKTROUND 1

class Frame;
class View : public ATL::CWindowImpl<View> {
 public:
  DECLARE_WND_CLASS(L"View")

  View();
  virtual ~View();

 public:
  virtual BOOL PreTranslateMessage(MSG* pMsg) {
    pMsg;
    return FALSE;
  }

 public:
  BEGIN_MSG_MAP(View)
  MESSAGE_HANDLER(WM_SIZE, OnSize)
  MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
  END_MSG_MAP()

 public:
  LRESULT OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                 BOOL& /*bHandled*/);

  // Prevent web area repaint
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms648055(v=vs.85).aspx
  LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/
                            ,
                            BOOL& /*bHandled*/) {
    return NO_ERASEBACKTROUND;
  }

 public:
  Frame* frame_;
};
}  // namespace gui

#endif  // GUI_VIEW_H_
