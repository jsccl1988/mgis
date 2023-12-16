
#ifndef _GT_FLASH_TOOL_H
#define _GT_FLASH_TOOL_H

#include "gt_basetool.h"

namespace _GroupTool {
class FlashTool : public BaseTool {
 public:
  FlashTool();
  virtual ~FlashTool();
  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *pToFollow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int Notify(long nMessage, ListenerMessage &param);

 protected:
  VectorLayer *m_pResultLayer;

  string m_strFlashStyle1;
  string m_strFlashStyle2;
  string m_strFlashStyle;
  eFlashMode m_flsMode;

  bool m_bFlash;
  bool m_bStyle1;
  double m_fScaleDelt;
};
}  // namespace _GroupTool

#endif  //_GT_FLASH_TOOL_H
