#ifndef _GT_TOOLCOMMANDRECEIVER_H
#define _GT_TOOLCOMMANDRECEIVER_H

#include "gt_basetool.h"
#include "smt_command.h"
using namespace core;

namespace _GroupTool {
class IACommandReceiver : public CommandReceiver {
 public:
  IACommandReceiver(LPRENDERDEVICE pRenderDevice, Map *pOperMap,
                    Feature *pFeature, IAType iaType);
  virtual ~IACommandReceiver();

 public:
  virtual bool Action(bool is_undo);

 public:
  IAType m_iaType;
  Feature *m_pFeature;
  Map *m_pOperMap;
  LPRENDERDEVICE m_pRenderDevice;
};
}  // namespace _GroupTool

#endif  //_GT_TOOLCOMMANDRECEIVER_H
