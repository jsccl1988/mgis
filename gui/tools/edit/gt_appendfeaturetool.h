#ifndef _GT_APPEND_FEATURE_H
#define _GT_APPEND_FEATURE_H

#include "gt_basetool.h"
#include "smt_command.h"
using namespace core;

namespace _GroupTool {
class AppendFeatureTool : public BaseTool {
 public:
  AppendFeatureTool();
  virtual ~AppendFeatureTool();
  int Create();
  int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *to_follow = NULL);
  int AuxDraw();
  int Timer();

 public:
  inline virtual void SetOperMap(Map *pOperMap);

 public:
  int Notify(long nMessage, ListenerMessage &param);

 public:
  int KeyDown(uint nChar, uint nRepCnt, uint nFlags);

 protected:
  void OnRetDelegate(int nRetType);

 protected:
  void OnInputPointFeature(ushort unType);
  void OnInputLineFeature(ushort unType);
  void OnInputRegionFeature(ushort unType);

 protected:
  void AppendPointFeature(ushort unType);
  void AppendChildImageFeature();
  void AppendTextFeature(const char *szAnno, float fangle);
  void AppendDotFeature();

  void AppendLineFeature(void);
  void AppendRegionFeature(void);

 protected:
  Geometry *m_pGeom;
  string m_strAnno;

 protected:
  CommandManager m_cmdMgr;
};
}  // namespace _GroupTool

#endif  //_GT_APPEND_FEATURE_H
