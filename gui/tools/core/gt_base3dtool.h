#ifndef _GT_BASE3DTOOL_H
#define _GT_BASE3DTOOL_H

#include "bl3d_scene.h"
#include "gt_basetool.h"
#include "gt_defs.h"
#include "rd3d_3drenderdevice.h"
#include "smt_bas_struct.h"
#include "smt_core.h"
#include "smt_msg.h"
#include "t_iatool.h"
#include "t_msg.h"


using namespace _3Drd;
using namespace core;
using namespace _IATool;

namespace _GroupTool {
class Base3DTool : public IATool {
 public:
  Base3DTool();
  virtual ~Base3DTool();

 public:
  virtual int Init(LP3DRENDERDEVICE p3DRenderDevice, Scene *pScene, HWND hWnd,
                   pfnToolCallBack pfnCallBack = NULL, void *to_follow = NULL);

 public:
  LP3DRENDERDEVICE GetRenderDevice(void) { return m_p3DRenderDevice; }
  Scene *GetScene(void) { return m_pScene; }

 protected:
  LP3DRENDERDEVICE m_p3DRenderDevice;
  Scene *m_pScene;
};
}  // namespace _GroupTool

#endif  //_GT_BASE3DTOOL_H