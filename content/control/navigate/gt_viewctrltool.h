#ifndef _GT_2DVIEW_CONTROL_TOOL_H
#define _GT_2DVIEW_CONTROL_TOOL_H

#include "gt_basetool.h"

namespace _GroupTool {
enum CursorType {
  CursorLoupePlus,
  CursorLoupeMinus,
  CursorMove,
  CursorIdentify
};

class ViewCtrlTool : public BaseTool {
 public:
  ViewCtrlTool();
  virtual ~ViewCtrlTool();

  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int Notify(long nMessage, ListenerMessage &param);

  int SetCursor(void);
  int LButtonDown(uint nFlags, lPoint point);
  int MouseMove(uint nFlags, lPoint point);
  int LButtonUp(uint nFlags, lPoint point);
  int RButtonDown(uint nFlags, lPoint point);
  int MouseWeel(uint nFlags, short zDelta, lPoint point);

 protected:
  void ZoomMove(short mouse_status, lPoint point);
  void ZoomIn(short mouse_status, lPoint point);
  void ZoomOut(short mouse_status, lPoint point);
  void ZoomRestore();
  void ZoomRefresh();

 protected:
  eViewMode m_viewMode;
  ushort m_usFlashed;

 protected:
  lPoint m_pntOrigin;
  lPoint m_pntCur;
  lPoint m_pntPrev;
  BOOL m_bCaptured;
  HCURSOR m_hCursors[4];
};
}  // namespace _GroupTool

#endif  //_GT_2DVIEW_CONTROL_TOOL_H
