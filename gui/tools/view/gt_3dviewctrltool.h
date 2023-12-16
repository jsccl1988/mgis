#ifndef _GT_VIEW3DTOOL_H
#define _GT_VIEW3DTOOL_H
#include "bl3d_object.h"
#include "gt_base3dtool.h"
#include "rd3d_camera.h"


using namespace _3DBase;

namespace _GroupTool {
enum eCursorType3D {
  CursorTraceBall,
  CursorSphereCamare,
  CursorFirstPerson,
};

class 3DViewCtrlTool : public Base3DTool {
 public:
  3DViewCtrlTool();
  virtual ~3DViewCtrlTool();

 public:
  int Init(LP3DRENDERDEVICE p3DRenderDevice, Scene * pScene, HWND hWnd,
           pfnToolCallBack pfnCallBack = NULL, void *pToFollow = NULL);
  int AuxDraw();
  int Timer();

 public:
  int Notify(long nMessage, ListenerMessage &param);

  int SetCursor(void);
  int LButtonDown(uint nFlags, lPoint point);
  int MouseMove(uint nFlags, lPoint point);
  int LButtonUp(uint nFlags, lPoint point);
  int MouseWeel(uint nFlags, short zDelta, lPoint point);
  int KeyDown(uint nChar, uint nRepCnt, uint nFlags);

 protected:
  void TrackballProv(lPoint pos, Vector3 & vec);

 protected:
  eView3DMode m_viewMode;
  PerspCamera *m_pCamera;
  Vector3 m_vOrgTarget, m_vOrgEye, m_vOrgUp;

  lPoint m_pntOrigin;
  lPoint m_pntPre;
  lPoint m_pntCur;
  bool m_bIsDrag;

  int m_nWinWidth;
  int m_nWinHeight;
  float m_fAngle;
  Vector3 m_vAxis;
  Vector3 m_vPrePos;

  HCURSOR m_hCursors[3];

  v3DObjectPtrs m_vSelTargetPtrs;
  3DObject * m_pLastTarget;
  Material m_matLastTarget;
  Material m_matSel;
};
}  // namespace _GroupTool

#endif  //_GT_VIEW3DTOOL_H