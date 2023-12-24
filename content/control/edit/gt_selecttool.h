#ifndef _GT_SELECTTOOL_H
#define _GT_SELECTTOOL_H

#include "gt_basetool.h"

namespace _GroupTool {
class SelectTool : public BaseTool {
 public:
  SelectTool();
  virtual ~SelectTool();

  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int KeyDown(uint nChar, uint nRepCnt, uint nFlags);

 public:
  int Notify(long nMessage, ListenerMessage &param);

 protected:
  void OnRetDelegate(int nRetType);
  void OnSetSelMode(void);

 protected:
  lPoint m_pntOrigin;
  lPoint m_pntCur;
  lPoint m_pntPrev;
  bool m_bCaptured;
  int m_nStep;

 protected:
  eSelectMode m_selMode;
  int m_nLayerFeaType;

  VectorLayer *m_pResultLayer;
  GQueryDesc m_gQDes;
  PQueryDesc m_pQDes;
  double m_dpMargin;
};
}  // namespace _GroupTool

#endif  //_GT_SELECTTOOL_H
