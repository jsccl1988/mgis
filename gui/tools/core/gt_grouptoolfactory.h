#ifndef _GT_GROUPTOOLFACTORY_H
#define _GT_GROUPTOOLFACTORY_H

#include "gt_base3dtool.h"
#include "gt_basetool.h"
#include "gt_defs.h"


using namespace _GroupTool;

enum GroupToolType {
  GTT_InputPoint,
  GTT_InputLine,
  GTT_InputRegion,
  GTT_AppendFeature,
  GTT_ViewControl,
  GTT_Flash,
  GTT_Select,
  GTT_WSViewControl,
};

enum GroupTool3DType {
  GTT_3DViewControl,
};

class CORE_EXPORT GroupToolFactory {
 public:
  // create
  static int CreateGroupTool(BaseTool*& pTool, GroupToolType type);
  static int CreateGroup3DTool(Base3DTool*& pTool, GroupTool3DType type);

  // destroy
  static int DestoryGroupTool(BaseTool*& pTool);
  static int DestoryGroup3DTool(Base3DTool*& pTool);

 private:
  GroupToolFactory(void);
  virtual ~GroupToolFactory(void);
};

#endif  //_GT_GROUPTOOLFACTORY_H