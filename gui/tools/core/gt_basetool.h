#ifndef _GT_BASETOOL_H
#define _GT_BASETOOL_H

#include "geo_geometry.h"
#include "gis_map.h"
#include "gt_defs.h"
#include "rd_renderdevice.h"
#include "smt_core.h"
#include "smt_msg.h"
#include "t_iatool.h"
#include "t_msg.h"


using namespace _GIS;
using namespace _Rd;
using namespace core;
using namespace _IATool;

#define SMT_IATOOL_MSG_KEY(lMessage) (SMT_MSG_KEY(lMessage, m_hWnd))

#define SMT_IATOOL_APPEND_MSG(lMessage) \
  { AppendMessage(SMT_IATOOL_MSG_KEY(lMessage)); }

namespace _GroupTool {
extern HINSTANCE g_hInstance;

//////////////////////////////////////////////////////////////////////////
class BaseTool : public IATool {
 public:
  BaseTool();
  virtual ~BaseTool();

 public:
  virtual int Init(LPRENDERDEVICE pMrdRenderDevice, Map *pOperMap, HWND hWnd,
                   pfnToolCallBack pfnCallBack = NULL, void *pToFollow = NULL);

  LPRENDERDEVICE GetRenderDevice(void) { return m_pRenderDevice; }

  virtual void SetOperMap(Map *pOperMap) { m_pOperMap = pOperMap; }
  virtual void GetOperMap(Map *&pOperMap) { pOperMap = m_pOperMap; }

  inline void SetToolStyleName(const char *name) {
    strcpy(m_szStyleName, name);
  }
  inline const char *GetToolStyleName(void) { return m_szStyleName; }

 public:
  virtual int KeyDown(uint nChar, uint nRepCnt, uint nFlags);

 protected:
  LPRENDERDEVICE m_pRenderDevice;
  Map *m_pOperMap;

  char m_szStyleName[MAX_STYLENAME_LENGTH];

  double m_fScaleDelt;
};
}  // namespace _GroupTool

#endif  //_GT_BASETOOL_H