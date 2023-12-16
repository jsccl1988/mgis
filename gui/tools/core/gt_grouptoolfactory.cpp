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
  int nRet = SMT_ERR_FAILURE;
  if (!pTool) {
    switch (type) {
      case GTT_InputPoint:
        pTool = new InputPointTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_InputLine:
        pTool = new InputLineTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_InputRegion:
        pTool = new InputRegionTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_AppendFeature:
        pTool = new AppendFeatureTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_ViewControl:
        pTool = new ViewCtrlTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_WSViewControl:
        pTool = new WSViewCtrlTool();
        nRet = SMT_ERR_NONE;
        break;

      case GTT_Select:
        pTool = new SelectTool();
        nRet = SMT_ERR_NONE;
        break;
      case GTT_Flash:
        pTool = new FlashTool();
        nRet = SMT_ERR_NONE;
        break;
      default:
        break;
    }
  }

  return nRet;
}

int GroupToolFactory::CreateGroup3DTool(Base3DTool*& pTool,
                                        GroupTool3DType type) {
  int nRet = SMT_ERR_FAILURE;
  if (!pTool) {
    switch (type) {
      case GTT_3DViewControl:
        pTool = new 3DViewCtrlTool();
        nRet = SMT_ERR_NONE;
        break;
      default:
        break;
    }
  }
  return nRet;
}

//////////////////////////////////////////////////////////////////////////
int GroupToolFactory::DestoryGroupTool(BaseTool*& pTool) {
  SMT_SAFE_DELETE(pTool);

  return SMT_ERR_NONE;
}

int GroupToolFactory::DestoryGroup3DTool(Base3DTool*& pTool) {
  SMT_SAFE_DELETE(pTool);

  return SMT_ERR_FAILURE;
}