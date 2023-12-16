#ifndef _GT_INIPUT_LINE_H
#define _GT_INIPUT_LINE_H

#include "gt_basetool.h"

namespace _GroupTool {
class InputLineTool : public BaseTool {
 public:
  InputLineTool();
  virtual ~InputLineTool();

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
  void AppendLineString(uint mouse_status, lPoint point);
  void AppendArc(uint mouse_status, lPoint point);
  void AppendRect(uint mouse_status, lPoint point);
  void AppendLinearRing(uint mouse_status, lPoint point);
  void AppendSpline(uint mouse_status, lPoint point);

  void EndAppendLine();

 protected:
  Geometry *m_pGeom;
  ushort m_appendType;

 private:
  lPoint m_pntOrigin;
  lPoint m_pntCur;
  lPoint m_pntPrev;
  bool m_bIsDrag;
  int m_nStep;
  bool m_bDelay;
};
}  // namespace _GroupTool

#endif  //_GT_INIPUT_LINE_H
