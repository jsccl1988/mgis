#ifndef _GT_INPUT_REGION_TOOL_H
#define _GT_INPUT_REGION_TOOL_H

#include "gt_basetool.h"

namespace _GroupTool {
class InputRegionTool : public BaseTool {
 public:
  InputRegionTool();
  virtual ~InputRegionTool();

  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *to_follow = NULL);
  int AuxDraw();

 public:
  int Notify(long nMessage, ListenerMessage &param);

  int LButtonDown(uint nFlags, lPoint point);
  int MouseMove(uint nFlags, lPoint point);
  int LButtonUp(uint nFlags, lPoint point);
  int RButtonDown(uint nFlags, lPoint point);
  int MouseWeel(uint nFlags, short zDelta, lPoint point);

 private:
  void AppendFan(uint mouse_status, lPoint point);
  void AppendRect(uint mouse_status, lPoint point);
  void AppendPolygon(uint mouse_status, lPoint point);

  void EndAppendRegion();

 protected:
  Geometry *m_pGeom;
  ushort m_appendType;

 private:
  double m_fScaleDelt;
  HCURSOR m_hCrossCursor;

  lPoint m_pntOrigin;
  lPoint m_pntCur;
  lPoint m_pntPrev;
  bool m_bIsDrag;
  bool m_bDelay;
  int m_nStep;
};
}  // namespace _GroupTool

#endif  //_GT_INPUT_REGION_TOOL_H
