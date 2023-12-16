#ifndef _GT_INPUT_POINT_H
#define _GT_INPUT_POINT_H

#include "gt_basetool.h"

namespace _GroupTool {
class InputPointTool : public BaseTool {
 public:
  InputPointTool();
  virtual ~InputPointTool();

  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *pToFollow = NULL);
  int AuxDraw();

 public:
  int Notify(long nMessage, ListenerMessage &param);

  int LButtonDown(uint nFlags, lPoint point);
  int MouseMove(uint nFlags, lPoint point);
  int LButtonUp(uint nFlags, lPoint point);
  int RButtonDown(uint nFlags, lPoint point);
  int MouseWeel(uint nFlags, short zDelta, lPoint point);

 private:
  void AppendChildImage(short mouse_status, lPoint point);
  void AppendText(short mouse_status, lPoint point);
  void AppendDot(short mouse_status, lPoint point);

  void EndAppendPoint();

 protected:
  Geometry *m_pGeom;
  ushort m_appendType;
  float m_fAngle;

 private:
  lPoint m_pntOrigin;
  lPoint m_pntCur;
  lPoint m_pntPrev;
  BOOL m_bIsDrag;
  bool m_bDelay;
};
}  // namespace _GroupTool

#endif  //_GT_INPUT_POINT_H
