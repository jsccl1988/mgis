#include "gt_grouptoolfactory.h"

#include "gt_3dviewctrltool.h"
#include "gt_appendfeaturetool.h"
#include "gt_basetool.h"
#include "gt_flashtool.h"
#include "gt_inputlinetool.h"
#include "gt_inputpointtool.h"
#include "gt_inputregiontool.h"
#include "gt_selecttool.h"
#include "gt_viewctrltool.h"
#include "gt_wsviewctrltool.h"

GroupToolFactory::GroupToolFactory(void) { ; }

GroupToolFactory::~GroupToolFactory(void) { ; }

int GroupToolFactory::CreateGroupTool(BaseTool*& pTool, GroupToolType type) {
  int nRet = ERR_FAILURE;
  if (!pTool) {
    switch (type) {
      case GTT_InputPoint:
        pTool = new InputPointTool();
        nRet = ERR_NONE;
        break;

      case GTT_InputLine:
        pTool = new InputLineTool();
        nRet = ERR_NONE;
        break;

      case GTT_InputRegion:
        pTool = new InputRegionTool();
        nRet = ERR_NONE;
        break;

      case GTT_AppendFeature:
        pTool = new AppendFeatureTool();
        nRet = ERR_NONE;
        break;

      case GTT_ViewControl:
        pTool = new ViewCtrlTool();
        nRet = ERR_NONE;
        break;

      case GTT_WSViewControl:
        pTool = new WSViewCtrlTool();
        nRet = ERR_NONE;
        break;

      case GTT_Select:
        pTool = new SelectTool();
        nRet = ERR_NONE;
        break;
      case GTT_Flash:
        pTool = new FlashTool();
        nRet = ERR_NONE;
        break;
      default:
        break;
    }
  }

  return nRet;
}

int GroupToolFactory::CreateGroup3DTool(Base3DTool*& pTool,
                                        GroupTool3DType type) {
  int nRet = ERR_FAILURE;
  if (!pTool) {
    switch (type) {
      case GTT_3DViewControl:
        pTool = new 3DViewCtrlTool();
        nRet = ERR_NONE;
        break;
      default:
        break;
    }
  }
  return nRet;
}

//////////////////////////////////////////////////////////////////////////
int GroupToolFactory::DestoryGroupTool(BaseTool*& pTool) {
  SAFE_DELETE(pTool);

  return ERR_NONE;
}

int GroupToolFactory::DestoryGroup3DTool(Base3DTool*& pTool) {
  SAFE_DELETE(pTool);

  return ERR_FAILURE;
}